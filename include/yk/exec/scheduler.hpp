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
#include "yk/exec/scheduler_traits.hpp"
#include "yk/exec/scheduler_stats.hpp"
#include "yk/exec/scheduler_stats_tracker.hpp"
#include "yk/exec/worker_pool.hpp"

#include "yk/concurrent_pool_gate.hpp"
#include "yk/concurrent_vector.hpp"

#include "yk/throwt.hpp"

#if YK_EXEC_DEBUG
#include <print>
#endif

#include <tuple>
#include <algorithm>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <stop_token>
#include <thread>

#include <ranges>
#include <concepts>
#include <type_traits>

namespace yk::exec {

template <
  producer_kind ProducerKind,
  consumer_kind ConsumerKind,
  ProducerInputRange ProducerInputRangeT,
  class T,
  class ProducerF,
  class ConsumerF,
  class TraitsT = scheduler_traits<ProducerKind, ConsumerKind, ProducerInputRangeT, T>
>
class scheduler
{
public:
  using traits_type        = TraitsT;
  using queue_type         = typename traits_type::queue_type;
  using producer_gate_type = typename traits_type::producer_gate_type;
  using consumer_gate_type = typename traits_type::consumer_gate_type;

  using producer_input_iterator = typename traits_type::producer_input_iterator;

  static_assert(Producer<ProducerF, T, ProducerInputRangeT, producer_gate_type>);
  static_assert(Consumer<ConsumerF, consumer_gate_type>);

  template <class ProducerF_, class ConsumerF_>
  scheduler(
    const std::shared_ptr<worker_pool>& worker_pool,
    ProducerF_&& producer_func,
    ConsumerF_&& consumer_func
  ) requires std::is_default_constructible_v<ProducerInputRangeT>
    : worker_pool_(worker_pool)
    , producer_func_(std::forward<ProducerF_>(producer_func))
    , consumer_func_(std::forward<ConsumerF_>(consumer_func))
  {
  }

  template <class ProducerF_, class ConsumerF_, class R>
  scheduler(
    const std::shared_ptr<worker_pool>& worker_pool,
    ProducerF_&& producer_func,
    ConsumerF_&& consumer_func,
    R&& producer_inputs
  )
    : worker_pool_(worker_pool)
    , producer_func_(std::forward<ProducerF_>(producer_func))
    , consumer_func_(std::forward<ConsumerF_>(consumer_func))
    , producer_inputs_(std::forward<R>(producer_inputs))
    , last_producer_input_it_(std::ranges::begin(producer_inputs_))
    , stats_(producer_inputs_)
  {
  }

  scheduler() = delete;
  scheduler(scheduler const&) = delete;
  scheduler(scheduler&&) = delete;
  scheduler& operator=(scheduler const&) = delete;
  scheduler& operator=(scheduler&&) = delete;

  // thread-safe, but must be called from the master thread
  ~scheduler()
  {
    this->abort();
  }

  // not thread-safe
  template <ProducerInputRange ProducerInputRangeT_>
  void set_producer_inputs(ProducerInputRangeT_&& r)
  {
    std::scoped_lock lock{stats_mtx_, producer_input_mtx_};

    if (stats_.is_running) {
      throwt<std::invalid_argument>(
        "Attempted to assign new producer inputs while scheduler is already running;"
        " this operation is unsupported due to implementation limitations"
      );
    }

    producer_inputs_ = std::forward<ProducerInputRangeT_>(r);
    last_producer_input_it_ = std::ranges::begin(producer_inputs_);
    stats_ = {producer_inputs_};
  }

  [[nodiscard]]
  auto get_producer_input_count() const
  {
    static_assert(std::ranges::sized_range<ProducerInputRangeT>);
    std::unique_lock lock{producer_input_mtx_};
    return std::ranges::size(producer_inputs_);
  }

  // not thread-safe
  void reset_same_inputs_for_next_execution()
  {
    std::scoped_lock lock{stats_mtx_, producer_input_mtx_};

    if (stats_.is_running) {
      throwt<std::invalid_argument>("scheduler is already running");
    }

    last_producer_input_it_ = std::ranges::begin(producer_inputs_);
    stats_ = {producer_inputs_};
  }

  // thread-safe
  [[nodiscard]]
  long long get_producer_chunk_size() const
  {
    std::unique_lock lock{producer_input_mtx_};
    return producer_chunk_size_;
  }

  // thread-safe
  void set_producer_chunk_size(long long chunk_size)
  {
    if (chunk_size < 1) {
      throwt<std::invalid_argument>("producer chunk size cannot be less than 1");
    }

    std::unique_lock lock{producer_input_mtx_};
    producer_chunk_size_ = chunk_size;
  }

  // not thread-safe
  [[nodiscard]]
  const std::shared_ptr<scheduler_stats_tracker>& get_stats_tracker() const noexcept
  {
    return stats_tracker_;
  }

  // not thread-safe
  void set_stats_tracker(std::unique_ptr<scheduler_stats_tracker> tracker)
  {
    stats_tracker_thread_.request_stop();
    if (stats_tracker_thread_.joinable()) {
      stats_tracker_thread_.join();
    }

    stats_tracker_ = std::move(tracker);
  }

  void launch_stats_tracker()
  {
    stats_tracker_thread_.request_stop();
    if (stats_tracker_thread_.joinable()) {
      stats_tracker_thread_.join();
    }

    if (!stats_tracker_) return;

    stats_tracker_->reset_first_tick();

    stats_tracker_thread_ = std::jthread{[this](std::stop_token stop_token) {
      try {
        while (!worker_pool_->stop_requested()) {
          std::unique_lock lock{stats_mtx_};

          const bool cv_ok = stats_tracker_cv_.wait_for(
            lock, stop_token, stats_tracker_->get_interval(),
            [this] {
              return stats_tracker_->interval_elapsed();
            }
          );

          const auto stats = stats_;
          lock.unlock();

          if (stop_token.stop_requested() || !cv_ok) {
            return;
          }

          stats_tracker_->tick(stats);
        }

      } catch (...) {
        stats_tracker_exception_ = std::current_exception();
      }
    }};
  }

  // thread-safe
  // intended to be used directly from user
  void halt_stats_tracker()
  {
    if (!stats_tracker_) return;
    stats_tracker_thread_.request_stop();
  }

  // not thread-safe
  void start()
  {
    {
      std::unique_lock lock{stats_mtx_};
      if (stats_.is_running) {
        throwt<std::invalid_argument>("Cannot start the scheduler while the jobs are running");
      }
      if (stats_.is_producer_input_processed_all()) {
        throwt<std::invalid_argument>("Cannot start the scheduler after a successful iteration. If this is your intended action, call: reset_same_inputs_for_next_execution()");
      }
    }

    queue_.set_capacity(queue_capacity_);
    queue_.reserve_capacity();

    launch_stats_tracker();

    worker_pool_->set_rethrow_exceptions_on_exit(true);

    worker_pool_->launch([this](const thread_id_t worker_id, std::stop_token stop_token) {
      this->fixed_consumer(worker_id, std::move(stop_token));
    });

    worker_pool_->launch([this](const thread_id_t worker_id, std::stop_token stop_token) {
      this->fixed_producer(worker_id, std::move(stop_token));
    });

    worker_pool_->launch_rest([this](const thread_id_t worker_id, std::stop_token stop_token) {
      this->dynamic_worker(worker_id, std::move(stop_token), detail::worker_mode_t::producer);
    });
  }

  // thread-safe, but must be called from the main thread
  void wait_for_all_tasks()
  {
    scheduler_stats last_stats;

    {
      std::unique_lock lock{stats_mtx_};
      task_done_cv_.wait(lock, worker_pool_->stop_token(), [this] {
        return stats_.is_all_task_done();
      });

      last_stats = stats_;
    }

#if YK_EXEC_DEBUG
    std::println("wait_for_all_tasks: notified");
#endif

    if (stats_tracker_) {
      stats_tracker_thread_.request_stop();

      if (stats_tracker_thread_.joinable()) {
        stats_tracker_thread_.join();
      }

      if (stats_tracker_exception_) {
        std::rethrow_exception(stats_tracker_exception_);
        stats_tracker_exception_ = {};
      }

      // always print tick at the end
      stats_tracker_->tick(last_stats);
    }

    if (worker_pool_->stop_requested()) {
#if YK_EXEC_DEBUG
      std::println("wait_for_all_tasks: stop requested");
#endif

      queue_.close();
      worker_pool_->set_rethrow_exceptions_on_exit(false);
      worker_pool_->rethrow_exceptions();
      return;

    } else {
#if YK_EXEC_DEBUG
      std::println("wait_for_all_tasks: task completed");
#endif

      auto const remaining_tasks = queue_.size();
      if (remaining_tasks != 0) {
        throwt<std::logic_error>("wait_for_all_tasks: queue is not empty ({})", remaining_tasks);
      }
    }
  }

  // thread-safe, but must be called from the master thread
  void abort()
  {
    queue_.close();
    worker_pool_->halt_and_clear();

    stats_tracker_thread_.request_stop();
    if (stats_tracker_thread_.joinable()) {
      stats_tracker_thread_.join();
    }
  }

private:
  template<bool NeedInfo>
  [[nodiscard]]
  std::conditional_t<NeedInfo, concurrent_pool_access_result, bool>
  do_worker_producer(const thread_id_t worker_id)
  {
    producer_input_iterator it_first, it_last;

    unsigned long long count;
    {
      std::scoped_lock lock{producer_input_mtx_, stats_mtx_};

      auto const producer_input_end = std::ranges::end(producer_inputs_);

      it_first = last_producer_input_it_;
      if (it_first == producer_input_end) {
        return {false}; // all producer done; need to switch to consumer
      }

      it_last = it_first;
      count = producer_chunk_size_ - static_cast<unsigned long long>(
        std::ranges::advance(it_last, producer_chunk_size_, producer_input_end)
      );
      last_producer_input_it_ = it_last;

      stats_.producer_input_consumed += count;

      if (it_last == producer_input_end) {
        stats_.set_producer_input_consumed_all();
      }
    }

    // ===== begin producer =====
    producer_gate_type gate{&queue_};

    for (auto it = it_first; it != it_last; ++it) {
      producer_func_(worker_id, *it, gate);
    }
    // ===== end producer =====

    {
      std::unique_lock lock{stats_mtx_};
      stats_.producer_input_processed += count;

      if constexpr (traits_type::is_multi_push) {
        stats_.producer_output += gate.count();
      } else {
        if (!gate.is_discarded()) ++stats_.producer_output;
      }

      if (stats_.is_producer_input_consumed_all() &&
        stats_.producer_input_processed >= stats_.producer_input_consumed
      ) {
        stats_.set_producer_input_processed_all();
      }

      // (reversed pattern; consumer outpaced our process)
      if (stats_.is_all_task_done()) {
        task_done_cv_.notify_all();
        return {false}; // need to switch to consumer
      }

      if (stats_.is_producer_input_consumed_all()) {
        return {false}; // need to switch to consumer
      }
    }

    //
    // producer is still required...
    //
    if constexpr (NeedInfo) {
      return {true, gate.last_pool_size()};

    } else {
      return true;
    }
  }

  template<bool NeedInfo>
  [[nodiscard]]
  std::conditional_t<NeedInfo, concurrent_pool_access_result, bool>
  do_worker_consumer(const thread_id_t worker_id)
  {
    // ===== begin consumer =====

    consumer_gate_type gate{&queue_};
    consumer_func_(worker_id, gate);

    // ===== end consumer =====

    {
      std::unique_lock lock{stats_mtx_};

      if constexpr (traits_type::is_multi_pop) {
        stats_.consumer_input_processed += gate.count();
      } else {
        if (!gate.is_discarded()) ++stats_.consumer_input_processed;
      }

      if (stats_.is_all_task_done()) {
        task_done_cv_.notify_all();
        return {false}; // need to switch to consumer
      }
    }

    //
    // consumer is still required...
    //
    if constexpr (NeedInfo) {
      return {true, gate.last_pool_size()};

    } else {
      return true;
    }
  }

  void fixed_producer(const thread_id_t worker_id, std::stop_token stop_token)
  {
    while (!stop_token.stop_requested()) {
      if (!do_worker_producer<false>(worker_id)) {
        break;
      }
    }
    if (stop_token.stop_requested()) {
      return;
    }

    while (!stop_token.stop_requested()) {
      if (!do_worker_consumer<false>(worker_id)) {
        break;
      }
    }
  }

  void fixed_consumer(const thread_id_t worker_id, std::stop_token stop_token)
  {
    while (!stop_token.stop_requested()) {
      if (!do_worker_consumer<false>(worker_id)) {
        break;
      }
    }
  }

  void dynamic_worker(const thread_id_t worker_id, std::stop_token stop_token, detail::worker_mode_t worker_mode)
  {
    while (!stop_token.stop_requested()) {
      if (worker_mode == detail::worker_mode_t::producer) {
        const auto [ok, size] = do_worker_producer<true>(worker_id);
        if (!ok) {
          //std::println("worker[{:2}] producer -> consumer", worker_id);
          worker_mode = detail::worker_mode_t::consumer;
          continue;
        }

        if (size >= queue_capacity_ * 0.9) {
          //std::println("worker[{:2}] producer -> consumer", worker_id);
          worker_mode = detail::worker_mode_t::consumer;
        }

      } else {  // Consumer
        const auto [ok, size] = do_worker_consumer<true>(worker_id);
        if (!ok) return;

        if (size <= queue_capacity_ * 0.1) {
          //std::println("worker[{:2}] consumer -> producer", worker_id);
          worker_mode = detail::worker_mode_t::producer;
        }
      }
    }
  }

  std::shared_ptr<worker_pool> worker_pool_;
  queue_type queue_;
  typename queue_type::size_type queue_capacity_ = 20 * 1024;

  // -----------------------------

  ProducerF producer_func_;
  ConsumerF consumer_func_;

  // -----------------------------

  mutable std::mutex producer_input_mtx_;
  ProducerInputRangeT producer_inputs_{};
  producer_input_iterator last_producer_input_it_ = std::ranges::end(producer_inputs_);
  long long producer_chunk_size_ = 0;

  // -----------------------------

  mutable std::mutex stats_mtx_;
  std::condition_variable_any task_done_cv_;
  scheduler_stats stats_{producer_inputs_};

  // -----------------------------

  std::unique_ptr<scheduler_stats_tracker> stats_tracker_;
  std::condition_variable_any stats_tracker_cv_;
  std::jthread stats_tracker_thread_;
  std::exception_ptr stats_tracker_exception_;
};


namespace detail {

template <
  producer_kind ProducerKind, consumer_kind ConsumerKind,
  class ProducerInputRangeT_, class T,
  class ProducerF_, class ConsumerF_,
  class TraitsT = scheduler_traits<ProducerKind, ConsumerKind, ProducerInputRangeT_, T>
>
using make_scheduler_t = scheduler<
  ProducerKind, ConsumerKind,
  std::decay_t<ProducerInputRangeT_>, T, std::decay_t<ProducerF_>, std::decay_t<ConsumerF_>,
  TraitsT
>;

} // detail


template <
  producer_kind ProducerKind, consumer_kind ConsumerKind,
  class T,
  class ProducerF_, class ConsumerF_, ProducerInputRange ProducerInputRangeT_
>
[[nodiscard]]
auto make_scheduler(
  const std::shared_ptr<yk::exec::worker_pool>& worker_pool,
  ProducerF_&& producer_func,
  ConsumerF_&& consumer_func,
  ProducerInputRangeT_&& producer_inputs
)
{
  return detail::make_scheduler_t<ProducerKind, ConsumerKind, ProducerInputRangeT_, T, ProducerF_, ConsumerF_>{
    worker_pool,
    std::forward<ProducerF_>(producer_func),
    std::forward<ConsumerF_>(consumer_func),
    std::forward<ProducerInputRangeT_>(producer_inputs)
  };
}

template <
  producer_kind ProducerKind, consumer_kind ConsumerKind,
  ProducerInputRange ProducerInputRangeT, class T,
  class TraitsT = scheduler_traits<ProducerKind, ConsumerKind, ProducerInputRangeT, T>,
  class ProducerF_, class ConsumerF_
>
[[nodiscard]]
auto make_scheduler(
  const std::shared_ptr<yk::exec::worker_pool>& worker_pool,
  ProducerF_&& producer_func,
  ConsumerF_&& consumer_func
) {
  return detail::make_scheduler_t<ProducerKind, ConsumerKind, ProducerInputRangeT, T, ProducerF_, ConsumerF_, TraitsT>{
    worker_pool,
    std::forward<ProducerF_>(producer_func),
    std::forward<ConsumerF_>(consumer_func)
  };
}

namespace detail {

template <class SchedulerTraits>
struct make_scheduler_with_traits_impl;

template <
  producer_kind ProducerKind, consumer_kind ConsumerKind,
  ProducerInputRange ProducerInputRangeT, class T, class BufT
>
struct make_scheduler_with_traits_impl<scheduler_traits<ProducerKind, ConsumerKind, ProducerInputRangeT, T, BufT>>
{
  template <class ProducerF_, class ConsumerF_, class... Args>
  static auto apply(
      const std::shared_ptr<yk::exec::worker_pool>& worker_pool,
      ProducerF_&& producer_func,
      ConsumerF_&& consumer_func,
      Args&&... args
  ) {
    return make_scheduler_t<ProducerKind, ConsumerKind, ProducerInputRangeT, T, ProducerF_, ConsumerF_, scheduler_traits<ProducerKind, ConsumerKind, ProducerInputRangeT, T, BufT>>{
      worker_pool,
      std::forward<ProducerF_>(producer_func),
      std::forward<ConsumerF_>(consumer_func),
      std::forward<Args>(args)...
    };
  }
};

} // detail

template <class SchedulerTraits, class... Args>
[[nodiscard]]
auto make_scheduler(Args&&... args)
{
  return detail::make_scheduler_with_traits_impl<SchedulerTraits>::apply(
    std::forward<Args>(args)...
  );
}

}  // yk::exec

#endif  // YK_HAS_EXEC_SCHEDULER

#endif
