#ifndef YK_EXEC_SCHEDULER_HPP
#define YK_EXEC_SCHEDULER_HPP

// yk::exec::scheduler requires std::stop_token support
#if __cpp_lib_jthread >= 201911L
#define YK_HAS_EXEC_SCHEDULER 1
#else
#define YK_HAS_EXEC_SCHEDULER 0
#endif

#if YK_HAS_EXEC_SCHEDULER

#include "yk/exec/debug.hpp"
#include "yk/exec/scheduler_stats.hpp"
#include "yk/exec/worker_pool.hpp"

#include "yk/concurrent_pool_gate.hpp"
#include "yk/concurrent_vector.hpp"

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <print>
#include <ranges>
#include <stop_token>
#include <thread>

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
  using producer_func = std::move_only_function<void(worker_id_t, const producer_input_value_type&, producer_gate_type&)>;

  using consumer_gate_type = yk::consumer_gate<queue_type>;
  using consumer_func = std::move_only_function<void(worker_id_t, T)>;
};

template <std::ranges::forward_range ProducerInputRange, class T>
class scheduler {
public:
  // clang-format off
  using producer_input_value_t    = std::ranges::range_value_t<ProducerInputRange>;
  using producer_input_iterator_t = std::ranges::iterator_t<ProducerInputRange>;

  using traits_type        = scheduler_traits<T, producer_input_value_t>;
  using queue_type         = typename traits_type::queue_type;
  using producer_gate_type = typename traits_type::producer_gate_type;
  using producer_func      = typename traits_type::producer_func;
  using consumer_gate_type = typename traits_type::consumer_gate_type;
  using consumer_func      = typename traits_type::consumer_func;
  // clang-format on

  static constexpr std::size_t default_queue_size = 10000;
  static constexpr long long default_producer_chunk_size_max = 100000;

  // not thread-safe
  explicit scheduler(const std::shared_ptr<worker_pool>& worker_pool)  //
      : worker_pool_(worker_pool) {
    queue_.reserve(default_queue_size);
  }

  // not thread-safe
  ~scheduler() {  //
    this->abort();
  }

  // not thread-safe
  template <class F>
  void set_producer(F&& f) {
    static_assert(Producer<F, T, producer_input_value_t, producer_gate_type>);
    producer_func_ = std::forward<F>(f);
  }

  // not thread-safe
  template <std::ranges::forward_range R>
  void set_producer_inputs(R&& producer_inputs) {
    producer_inputs_ = std::forward<R>(producer_inputs);
    last_producer_input_it_ = producer_inputs_.begin();
    producer_input_total_ = static_cast<long long>(producer_inputs_.size());
  }

  // thread-safe while processing the same input
  [[nodiscard]]
  long long get_producer_input_total() const noexcept {
    return producer_input_total_;
  }

  // not thread-safe
  [[nodiscard]]
  long long get_producer_chunk_size() const noexcept {
    return producer_chunk_size_;
  }

  // not thread-safe
  void set_producer_chunk_size(long long chunk_size) {
    if (chunk_size <= 0) {
      throw std::length_error{"producer_chunk_size must be greater than 0"};
    }
    producer_chunk_size_ = chunk_size;
  }

  // not thread-safe
  template <class F>
  void set_consumer(F&& f) {
    static_assert(Consumer<F, T>);
    consumer_func_ = std::forward<F>(f);
  }

  // not thread-safe
  [[nodiscard]]
  const std::shared_ptr<scheduler_stats_tracker>& get_stats_tracker() const noexcept {
    return stats_tracker_;
  }

  // not thread-safe
  void set_stats_tracker(const std::shared_ptr<scheduler_stats_tracker>& tracker) {
    halt_stats_tracker();

    stats_tracker_ = tracker;

    stats_tracker_thread_ = std::jthread{[this](std::stop_token stop_token) {
      while (!worker_pool_->stop_requested()) {
        std::unique_lock lock{pool_stats_mtx_};

        const bool cv_ok = stats_tracker_cv_.wait_for(         //
            lock, stop_token, stats_tracker_->get_interval(),  //
            [this] { return stats_tracker_->interval_elapsed(); });

        const auto stats = stats_;
        lock.unlock();

        // always print tick at the end
        stats_tracker_->tick(stats);

        if (stop_token.stop_requested() || !cv_ok) {
          return;
        }
      }
    }};
  }

  // thread-safe
  void halt_stats_tracker() {
    if (!stats_tracker_) return;

    stats_tracker_thread_.request_stop();

    if (stats_tracker_thread_.joinable()) {
      stats_tracker_thread_.join();
    }
  }

  // not thread-safe
  void start() {
    if (producer_chunk_size_ <= 0) {
      producer_chunk_size_ =                                                    //
          std::clamp(producer_input_total_ / worker_pool_->get_worker_limit(),  //
                     1ll, default_producer_chunk_size_max);
    }

    worker_pool_->halt_and_clear();

    worker_pool_->launch([this](const worker_id_t worker_id, std::stop_token stop_token) {  //
      fixed_consumer(worker_id, std::move(stop_token));
    });

    worker_pool_->launch([this](const worker_id_t worker_id, std::stop_token stop_token) {  //
      fixed_producer(worker_id, std::move(stop_token));
    });

    worker_pool_->launch_rest([this](const worker_id_t worker_id, std::stop_token stop_token) {  //
      worker(worker_id, std::move(stop_token), worker_mode_t::producer);
    });
  }

  // thread-safe
  void wait_for_all_tasks() {
    {
      std::unique_lock lock{pool_stats_mtx_};
      task_done_cv_.wait(lock, worker_pool_->stop_token(), [this] {
        return stats_.producer_input_processed >= producer_input_total_ &&  //
               stats_.consumer_input_processed >= stats_.producer_output;
      });

      if (worker_pool_->stop_requested()) {
        lock.unlock();
        queue_.close();
        std::println("interrupted! remaining items: {}", queue_.size());
        return;
      }

      stats_.validate_counter_consistency(producer_input_total_);
    }

    halt_stats_tracker();
  }

  // thread-safe
  void abort() {
    halt_stats_tracker();
    queue_.close();
  }

private:
  [[nodiscard]]
  bool do_worker_producer(const worker_id_t worker_id) {
    long long count = producer_chunk_size_;

    producer_input_iterator_t it_first, it_last;
    {
      std::unique_lock lock{producer_input_mtx_};
      it_first = last_producer_input_it_;

      if (const auto len = static_cast<long long>(std::ranges::distance(it_first, producer_inputs_.end()));  //
          len < producer_chunk_size_) {
        count = len;
        it_last = producer_inputs_.end();

      } else {
        it_last = std::ranges::next(it_first, producer_chunk_size_);
      }

      last_producer_input_it_ = it_last;
    }

#if YK_EXEC_DEBUG
    {
      std::unique_lock lock{pool_stats_mtx_};
      stats_.producer_input_consumed += count;
    }
#endif

    producer_gate_type gate{&queue_};

    for (auto it = it_first; it != it_last; ++it) {
      producer_func_(worker_id, *it, gate);
    }

    {
      std::unique_lock lock{pool_stats_mtx_};
      stats_.producer_input_processed += count;
      stats_.producer_output += gate.count();

      if (stats_.producer_input_processed >= producer_input_total_) {
        if (stats_.consumer_input_processed >= stats_.producer_output) {
          // all tasks done (reversed pattern; consumer outpaced our process)
          task_done_cv_.notify_all();
          return false;
        }

        // no more task for producer; need to switch
        return false;
      }
    }

    return true;
  }

  [[nodiscard]]
  bool do_worker_consumer(const worker_id_t worker_id) {
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

      if (stats_.producer_input_processed >= producer_input_total_ &&  //
          stats_.consumer_input_processed >= stats_.producer_output) {
        task_done_cv_.notify_all();
        return false;
      }
    }

    return true;
  }

  void fixed_producer(const worker_id_t worker_id, std::stop_token stop_token) {
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

  void fixed_consumer(const worker_id_t worker_id, std::stop_token stop_token) {
    while (!stop_token.stop_requested()) {
      if (!do_worker_consumer(worker_id)) {
        break;
      }
    }
  }

  void worker(const worker_id_t worker_id, std::stop_token stop_token, worker_mode_t worker_mode) {
    while (!stop_token.stop_requested()) {
      if (worker_mode == worker_mode_t::producer) {
        if (!do_worker_producer(worker_id)) {
          // std::println("worker[{:2}] producer -> consumer", worker_id);
          worker_mode = worker_mode_t::consumer;
          continue;
        }

        const auto size_info = queue_.size_info();

        if (size_info.size >= size_info.capacity * 0.9) {
          // std::println("worker[{:2}] producer -> consumer", worker_id);
          worker_mode = worker_mode_t::consumer;
        }

      } else {  // Consumer
        if (!do_worker_consumer(worker_id)) {
          return;
        }

        const auto size_info = queue_.size_info();

        if (size_info.size <= size_info.capacity * 0.1) {
          // std::println("worker[{:2}] consumer -> producer", worker_id);
          worker_mode = worker_mode_t::producer;
        }
      }
    }
  }

  std::shared_ptr<worker_pool> worker_pool_;

  queue_type queue_;

  // -----------------------------

  producer_func producer_func_;
  consumer_func consumer_func_;

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

  std::shared_ptr<scheduler_stats_tracker> stats_tracker_;
  std::jthread stats_tracker_thread_;
  std::condition_variable_any stats_tracker_cv_;
};

}  // namespace yk::exec

#endif  // YK_HAS_EXEC_SCHEDULER

#endif
