#ifndef YK_EXEC_SCHEDULER_HPP
#define YK_EXEC_SCHEDULER_HPP

#include <version>

// yk::exec::scheduler requires std::stop_token support
#if __cpp_lib_jthread >= 201911L
#define YK_HAS_EXEC_SCHEDULER 1
#else
#define YK_HAS_EXEC_SCHEDULER 0
#endif

#if YK_HAS_EXEC_SCHEDULER

#include "yk/exec/debug.hpp"
#include "yk/exec/scheduler_stats.hpp"

#include "yk/concurrent_pool_gate.hpp"
#include "yk/concurrent_vector.hpp"

#include <vector>
#include <algorithm>
#include <concepts>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <ranges>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <type_traits>

namespace yk::exec {

enum struct worker_mode_t : bool {
  producer,
  consumer,
};

template <class F, class T, class ProducerInputT, class GateT>
concept Producer = std::invocable<F, worker_id_t, const ProducerInputT&, GateT&>;

template <class F, class T>
concept Consumer = std::invocable<F, worker_id_t, T>;

template <class T, class ProducerInputValueT>
struct scheduler_traits {
  using queue_type = yk::concurrent_mpmc_vector<T>;

  using producer_input_value_type = ProducerInputValueT;
  using producer_gate_type = yk::counted_producer_gate<queue_type>;
  using consumer_gate_type = yk::consumer_gate<queue_type>;
};

template <
  std::ranges::forward_range ProducerInputRange,
  class T,
  class ProducerF,
  class ConsumerF
>
class scheduler {
public:
  using producer_input_value_type = std::ranges::range_value_t<ProducerInputRange>;
  using producer_input_iterator_t = std::ranges::iterator_t<ProducerInputRange>;

  using traits_type        = scheduler_traits<T, producer_input_value_type>;
  using queue_type         = typename traits_type::queue_type;
  using producer_gate_type = typename traits_type::producer_gate_type;
  using consumer_gate_type = typename traits_type::consumer_gate_type;

  static_assert(Producer<ProducerF, T, producer_input_value_type, producer_gate_type>);
  static_assert(Consumer<ConsumerF, T>);

  template<class ProducerF_, class ConsumerF_, class R>
  scheduler(
    const std::shared_ptr<worker_pool>& worker_pool,
    ProducerF_&& producer_func,
    ConsumerF_&& consumer_func,
    R&& producer_inputs
  )
    : producer_func_(std::forward<ProducerF_>(producer_func))
    , consumer_func_(std::forward<ConsumerF_>(consumer_func))
    , producer_inputs_(std::forward<R>(producer_inputs))
  {
    last_producer_input_it_ = producer_inputs_.begin();
    producer_input_total_ = static_cast<long long>(producer_inputs_.size());
  }

  ~scheduler()
  {
    this->abort();
  }

  void start()
  {
    producer_chunk_size_ = 1000;

    threads_.emplace_back([this] {
      fixed_consumer(0, stop_source_.get_token());
    });

    for (worker_id_t worker_id = 0; worker_id < 30; ++worker_id) {
      threads_.emplace_back([this, worker_id] {
        fixed_producer(worker_id, stop_source_.get_token());
      });
    }
  }

  void wait_for_all_tasks()
  {
    {
      std::unique_lock lock{pool_stats_mtx_};
      task_done_cv_.wait(
        lock,
        [this] {
          return
            stats_.producer_input_processed >= producer_input_total_ &&
            stats_.consumer_input_processed >= stats_.producer_output;
        }
      );

      std::println("wait_for_all_tasks: task done notified");
      stop_source_.request_stop();
      queue_.close();
    }

    for (auto& thread : threads_) {
        thread.join();
    }
    threads_.clear();
    std::println("wait_for_all_tasks: thread joined");
  }

  void abort()
  {
    queue_.close();
  }

private:
  [[nodiscard]]
  bool do_worker_producer(const worker_id_t worker_id)
  {
    long long count = producer_chunk_size_;

    producer_input_iterator_t it_first = producer_inputs_.end();
    producer_input_iterator_t it_last = producer_inputs_.end();
    producer_input_iterator_t it_tmp = producer_inputs_.end();
    {
      std::unique_lock lock{producer_input_mtx_};
      it_first = last_producer_input_it_;
      if (it_first == producer_inputs_.end()) {
          return false;
      }

      it_last = std::ranges::next(it_first, producer_chunk_size_, producer_inputs_.end());
      count = static_cast<long long>(std::ranges::distance(it_first, it_last));

      last_producer_input_it_ = it_last;
    }

#if YK_EXEC_DEBUG
    {
      std::unique_lock lock{pool_stats_mtx_};
      stats_.producer_input_consumed += count;
    }
#endif

    producer_gate_type gate{&queue_};

    auto it = producer_inputs_.begin();

    for (long long i = 0; i < count; ++i) {
      auto current_it = it++;
      auto tmp = *current_it;
      producer_func_(worker_id, tmp, gate);
    }

    {
      std::unique_lock lock{pool_stats_mtx_};
      stats_.producer_input_processed += count;
      stats_.producer_output += gate.count();

      if (stats_.producer_input_processed >= producer_input_total_ &&
        stats_.consumer_input_processed >= stats_.producer_output
      ) {
        // all tasks done (reversed pattern; consumer outpaced our process)
        task_done_cv_.notify_all();
        return false;
      }
    }

    if (it_last == producer_inputs_.end()) {
      // no more task for producer; need to switch
      return false;
    }

    return true;
  }

  [[nodiscard]]
  bool do_worker_consumer(const worker_id_t worker_id)
  {
    T value;
    const bool got_value = queue_.pop_wait(value);
    if (!got_value) {
      return false;
    }

#if YK_EXEC_DEBUG
    {
      std::unique_lock lock{pool_stats_mtx_};
      ++stats_.consumer_input_consumed;
    }
#endif

    consumer_func_(worker_id, std::move(value));

    {
      std::unique_lock lock{pool_stats_mtx_};
      ++stats_.consumer_input_processed;

      if (
        stats_.producer_input_processed >= producer_input_total_ &&
        stats_.consumer_input_processed >= stats_.producer_output
      ) {
        task_done_cv_.notify_all();
        return false;
      }
    }

    return true;
  }

  void fixed_producer(const worker_id_t worker_id, std::stop_token stop_token)
  {
    while (!stop_token.stop_requested()) {
      if (!do_worker_producer(worker_id)) {
        break;
      }
    }
    if (stop_token.stop_requested()) {
      return;
    }

    while (!stop_token.stop_requested()) {
      if (!do_worker_consumer(worker_id)) {
        break;
      }
    }
  }

  void fixed_consumer(const worker_id_t worker_id, std::stop_token stop_token)
  {
    while (!stop_token.stop_requested()) {
      if (!do_worker_consumer(worker_id)) {
        break;
      }
    }
  }

  std::vector<std::thread> threads_;

  queue_type queue_;

  // -----------------------------

  ProducerF producer_func_;
  ConsumerF consumer_func_;

  // -----------------------------

  ProducerInputRange producer_inputs_;
  long long producer_input_total_ = 0;
  long long producer_chunk_size_ = 0;

  std::mutex producer_input_mtx_;
  producer_input_iterator_t last_producer_input_it_ = producer_inputs_.end();

  // -----------------------------

  mutable std::mutex pool_stats_mtx_;
  std::condition_variable_any task_done_cv_;
  scheduler_stats stats_{};

  std::stop_source stop_source_;
};

template <std::ranges::forward_range ProducerInputRange, class T, class ProducerF_, class ConsumerF_, class R>
[[nodiscard]]
auto make_scheduler(
    const std::shared_ptr<yk::exec::worker_pool>& worker_pool,
    ProducerF_&& producer_func,
    ConsumerF_&& consumer_func,
    R&& producer_inputs
) {
  return scheduler<ProducerInputRange, T, std::decay_t<ProducerF_>, std::decay_t<ConsumerF_>>{
    worker_pool,
    std::forward<ProducerF_>(producer_func),
    std::forward<ConsumerF_>(consumer_func),
    std::forward<R>(producer_inputs)
  };
}

}  // namespace yk::exec

#endif  // YK_HAS_EXEC_SCHEDULER

#endif
