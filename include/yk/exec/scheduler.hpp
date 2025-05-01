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
#include "yk/exec/scheduler_delta_stats.hpp"
#include "yk/exec/scheduler_stats_tracker.hpp"
#include "yk/exec/worker_pool.hpp"
#include "yk/exec/queue_gate.hpp"
#include "yk/exec/queue_traits.hpp"

#include "yk/arch.hpp"
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

namespace detail {

template <class TraitsT, bool need_stop_token_for_cancel>
struct scheduler_base
{
  using queue_type         = typename TraitsT::queue_type;
  using producer_gate_type = typename TraitsT::producer_gate_type;
  using consumer_gate_type = typename TraitsT::consumer_gate_type;

protected:
  [[nodiscard]]
  producer_gate_type make_producer_gate(queue_type* queue) noexcept
  {
    return producer_gate_type{queue};
  }

  [[nodiscard]]
  consumer_gate_type make_consumer_gate(queue_type* queue) noexcept
  {
    return consumer_gate_type{queue};
  }
};

template <class TraitsT>
struct scheduler_base<TraitsT, true>
{
  using queue_type         = typename TraitsT::queue_type;
  using producer_gate_type = typename TraitsT::producer_gate_type;
  using consumer_gate_type = typename TraitsT::consumer_gate_type;

protected:
  [[nodiscard]]
  producer_gate_type make_producer_gate(queue_type* queue) noexcept
  {
    return producer_gate_type{queue, queue_stop_source_.get_token()};
  }

  [[nodiscard]]
  consumer_gate_type make_consumer_gate(queue_type* queue) noexcept
  {
    return consumer_gate_type{queue, queue_stop_source_.get_token()};
  }

  std::stop_source queue_stop_source_;
};

} // detail


template <
  producer_kind ProducerKind,
  consumer_kind ConsumerKind,
  ProducerInputRange ProducerInputRangeT,
  class QueueT,
  class ProducerF,
  class ConsumerF,
  class TraitsT = scheduler_traits<ProducerKind, ConsumerKind, ProducerInputRangeT, QueueT>
>
class scheduler : detail::scheduler_base<TraitsT, queue_traits<QueueT>::need_stop_token_for_cancel>
{
  using base_type = detail::scheduler_base<TraitsT, queue_traits<QueueT>::need_stop_token_for_cancel>;

public:
  using queue_type         = QueueT;
  using queue_traits_type  = queue_traits<QueueT>;
  using traits_type        = TraitsT;
  using producer_gate_type = typename base_type::producer_gate_type;
  using consumer_gate_type = typename base_type::consumer_gate_type;

  using producer_input_iterator = typename traits_type::producer_input_iterator;

  static_assert(Producer<ProducerF, ProducerInputRangeT, producer_gate_type>);
  static_assert(Consumer<ConsumerF, consumer_gate_type>);

  // ----------------------------------------

  // lazy producer input
  template <class ProducerF_, class ConsumerF_, class... QueueArgs>
    requires std::is_default_constructible_v<ProducerInputRangeT> && std::constructible_from<QueueT, QueueArgs...>
  scheduler(
    const std::shared_ptr<worker_pool>& worker_pool,
    ProducerF_&& producer_func,
    ConsumerF_&& consumer_func,
    QueueArgs&&... queue_args
  )
    : worker_pool_(worker_pool)
    , producer_func_(std::forward<ProducerF_>(producer_func))
    , consumer_func_(std::forward<ConsumerF_>(consumer_func))
    , queue_(std::forward<QueueArgs>(queue_args)...)
  {
  }

  // initialize with producer input
  template <class ProducerF_, class ConsumerF_, class R, class... QueueArgs>
    requires std::constructible_from<ProducerInputRangeT, R> && std::constructible_from<QueueT, QueueArgs...>
  scheduler(
    const std::shared_ptr<worker_pool>& worker_pool,
    ProducerF_&& producer_func,
    ConsumerF_&& consumer_func,
    R&& producer_inputs,
    QueueArgs&&... queue_args
  )
    : worker_pool_(worker_pool)
    , producer_func_(std::forward<ProducerF_>(producer_func))
    , consumer_func_(std::forward<ConsumerF_>(consumer_func))
    , producer_inputs_(std::forward<R>(producer_inputs))
    , last_producer_input_it_(std::ranges::begin(producer_inputs_))
    , stats_(producer_inputs_)
    , queue_(std::forward<QueueArgs>(queue_args)...)
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
  const scheduler_stats_tracker* get_stats_tracker() const noexcept
  {
    return stats_tracker_.get();
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

    launch_stats_tracker();

    worker_pool_->set_rethrow_exceptions_on_exit(true);

    worker_pool_->launch([this](const thread_index_t worker_id, std::stop_token stop_token) {
      this->fixed_consumer(worker_id, std::move(stop_token));
    });

    worker_pool_->launch([this](const thread_index_t worker_id, std::stop_token stop_token) {
      this->fixed_producer(worker_id, std::move(stop_token));
    });

    worker_pool_->launch_rest([this](const thread_index_t worker_id, std::stop_token stop_token) {
      this->dynamic_worker(worker_id, std::move(stop_token), worker_mode_t::producer);
    });
  }

  // thread-safe, but must be called from the main thread
  void wait_for_all_tasks()
  {
    scheduler_stats prev_stats;

    {
      std::unique_lock lock{stats_mtx_};
      task_done_cv_.wait(lock, worker_pool_->stop_token(), [this] {
        return stats_.is_all_task_done();
      });

      prev_stats = stats_;
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
      stats_tracker_->tick(prev_stats);
    }

    if (worker_pool_->stop_requested()) {
#if YK_EXEC_DEBUG
      std::println("wait_for_all_tasks: stop requested");
#endif

      //queue_.close(); // FIXME: lockfree migration
      worker_pool_->set_rethrow_exceptions_on_exit(false);
      worker_pool_->rethrow_exceptions();
      return;

    } else {
#if YK_EXEC_DEBUG
      std::println("wait_for_all_tasks: task completed");
#endif

      // FIXME: lockfree migration
      //auto const remaining_tasks = queue_.size();
      //if (remaining_tasks != 0) {
      //  throwt<std::logic_error>("wait_for_all_tasks: queue is not empty ({})", remaining_tasks);
      //}
    }
  }

  // thread-safe, but must be called from the master thread
  void abort()
  {
    // FIXME: lockfree migration
    //queue_.close();
    worker_pool_->halt_and_clear();

    stats_tracker_thread_.request_stop();
    if (stats_tracker_thread_.joinable()) {
      stats_tracker_thread_.join();
    }
  }


  [[nodiscard]] queue_type const& queue() const noexcept { return queue_; }
  [[nodiscard]] queue_type& queue() noexcept { return queue_; }

private:
  template <bool NeedInfo>
  [[nodiscard]]
  std::conditional_t<NeedInfo, std::pair<bool, double>, bool>
  do_worker_producer(const thread_index_t worker_id)
  {
    producer_input_iterator it_first, it_last;

    unsigned long long count;
    {
      std::scoped_lock lock{producer_input_mtx_, stats_mtx_};

      auto const producer_input_end = std::ranges::end(producer_inputs_);

      it_first = last_producer_input_it_;
      if (it_first == producer_input_end) {
        return {}; // all producer done; need to switch to consumer
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
    auto gate = this->make_producer_gate(&queue_);

#if YK_EXEC_DEBUG
    std::chrono::nanoseconds process_time{};
#endif

    for (auto it = it_first; it != it_last; ++it) {
#if YK_EXEC_DEBUG
      auto const start_time = std::chrono::steady_clock::now();
#endif

      producer_func_(worker_id, *it, gate);

#if YK_EXEC_DEBUG
      process_time += std::chrono::steady_clock::now() - start_time;
#endif
    }

    // ===== end producer =====

    thread_local scheduler_stats prev_stats{};
    double p_c_ratio;

    {
      std::unique_lock lock{stats_mtx_};

#if YK_EXEC_DEBUG
      stats_.producer_time += process_time;
      stats_.queue_overhead += gate.elapsed_time();
#endif

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
        return {}; // need to switch to consumer
      }

      if (stats_.is_producer_input_consumed_all()) {
        return {}; // need to switch to consumer
      }

      p_c_ratio = stats_.consumer_input_processed == 0 ? 0.0
        : double(stats_.producer_output) / stats_.consumer_input_processed;

      prev_stats = stats_;
    }

    //
    // producer is still required...
    //
    if constexpr (NeedInfo) {
      return {true, p_c_ratio};
    } else {
      return true;
    }
  }

  template <bool NeedInfo>
  [[nodiscard]]
  std::conditional_t<NeedInfo, std::pair<bool, double>, bool>
  do_worker_consumer(const thread_index_t worker_id)
  {
    // ===== begin consumer =====

    auto gate = this->make_consumer_gate(&queue_);

#if YK_EXEC_DEBUG
    auto const start_time = std::chrono::steady_clock::now();
#endif

    consumer_func_(worker_id, gate);

#if YK_EXEC_DEBUG
    auto const process_time = std::chrono::steady_clock::now() - start_time;
#endif

    // ===== end consumer =====

    thread_local scheduler_stats prev_stats{};
    double p_c_ratio;

    {
      std::unique_lock lock{stats_mtx_};

#if YK_EXEC_DEBUG
      stats_.consumer_time += process_time;
      stats_.queue_overhead += gate.elapsed_time();
#endif

      if constexpr (traits_type::is_multi_pop) {
        stats_.consumer_input_processed += gate.count();
      } else {
        if (!gate.is_discarded()) ++stats_.consumer_input_processed;
      }

      if (stats_.is_all_task_done()) {
        task_done_cv_.notify_all();
        return {}; // need to switch to consumer
      }

      p_c_ratio = stats_.consumer_input_processed == 0 ? 0.0
        : double(stats_.producer_output) / stats_.consumer_input_processed;

      prev_stats = stats_;
    }

    //
    // consumer is still required...
    //
    if constexpr (NeedInfo) {
      return {true, p_c_ratio};

    } else {
      return true;
    }
  }

  void fixed_producer(const thread_index_t worker_id, std::stop_token stop_token)
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

  void fixed_consumer(const thread_index_t worker_id, std::stop_token stop_token)
  {
    while (!stop_token.stop_requested()) {
      if (!do_worker_consumer<false>(worker_id)) {
        break;
      }
    }
  }

  void dynamic_worker(const thread_index_t worker_id, std::stop_token stop_token, worker_mode_t worker_mode)
  {
    while (!stop_token.stop_requested()) {
      if (worker_mode == worker_mode_t::producer) {
        const auto [ok, p_c_ratio] = do_worker_producer<true>(worker_id);
        if (!ok) {
          //std::println("worker[{:2}] producer -> consumer", worker_id);
          worker_mode = worker_mode_t::consumer;
          continue;
        }

        //std::println("p_c_ratio: {}", p_c_ratio);

        if (p_c_ratio >= 1.0) {
          //std::println("worker[{:2}] producer -> consumer", worker_id);
          worker_mode = worker_mode_t::consumer;
        }

      } else {  // Consumer
        const auto [ok, p_c_ratio] = do_worker_consumer<true>(worker_id);
        if (!ok) return;

        //std::println("p_c_ratio: {}", p_c_ratio);

        if (p_c_ratio < 1.0) {
          //std::println("worker[{:2}] consumer -> producer", worker_id);
          worker_mode = worker_mode_t::producer;
        }
      }
    }
  }

YK_FORCEALIGN_BEGIN
  std::shared_ptr<worker_pool> worker_pool_;
  ProducerF producer_func_;
  ConsumerF consumer_func_;

  // -----------------------------

  alignas(yk::hardware_destructive_interference_size) queue_type queue_;

  // -----------------------------

  alignas(yk::hardware_destructive_interference_size) mutable std::mutex producer_input_mtx_;
  ProducerInputRangeT producer_inputs_{};
  producer_input_iterator last_producer_input_it_ = std::ranges::end(producer_inputs_);
  long long producer_chunk_size_ = 0;

  // -----------------------------

  alignas(yk::hardware_destructive_interference_size) mutable std::mutex stats_mtx_;
  alignas(yk::hardware_destructive_interference_size) std::condition_variable_any task_done_cv_;
  alignas(yk::hardware_destructive_interference_size) scheduler_stats stats_{producer_inputs_};

  // -----------------------------

  std::unique_ptr<scheduler_stats_tracker> stats_tracker_;
  alignas(yk::hardware_destructive_interference_size) std::condition_variable_any stats_tracker_cv_;
  std::jthread stats_tracker_thread_;
  std::exception_ptr stats_tracker_exception_;
YK_FORCEALIGN_END
};


namespace detail {

template <
  producer_kind ProducerKind, consumer_kind ConsumerKind,
  class ProducerInputRangeT_, class QueueT,
  class ProducerF_, class ConsumerF_,
  class TraitsT = scheduler_traits<ProducerKind, ConsumerKind, ProducerInputRangeT_, QueueT>
>
using make_scheduler_t = scheduler<
  ProducerKind, ConsumerKind,
  std::decay_t<ProducerInputRangeT_>, QueueT, std::decay_t<ProducerF_>, std::decay_t<ConsumerF_>,
  TraitsT
>;

} // detail


template <
  producer_kind ProducerKind, consumer_kind ConsumerKind,
  class QueueT,
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
  return detail::make_scheduler_t<ProducerKind, ConsumerKind, ProducerInputRangeT_, QueueT, ProducerF_, ConsumerF_>{
    worker_pool,
    std::forward<ProducerF_>(producer_func),
    std::forward<ConsumerF_>(consumer_func),
    std::forward<ProducerInputRangeT_>(producer_inputs)
  };
}

template <
  producer_kind ProducerKind, consumer_kind ConsumerKind,
  ProducerInputRange ProducerInputRangeT, class QueueT,
  class TraitsT = scheduler_traits<ProducerKind, ConsumerKind, ProducerInputRangeT, QueueT>,
  class ProducerF_, class ConsumerF_
>
[[nodiscard]]
auto make_scheduler(
  const std::shared_ptr<yk::exec::worker_pool>& worker_pool,
  ProducerF_&& producer_func,
  ConsumerF_&& consumer_func
) {
  return detail::make_scheduler_t<ProducerKind, ConsumerKind, ProducerInputRangeT, QueueT, ProducerF_, ConsumerF_, TraitsT>{
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
  ProducerInputRange ProducerInputRangeT, class QueueT
>
struct make_scheduler_with_traits_impl<scheduler_traits<ProducerKind, ConsumerKind, ProducerInputRangeT, QueueT>>
{
  template <class ProducerF_, class ConsumerF_, class... Args>
  static auto apply(
      const std::shared_ptr<yk::exec::worker_pool>& worker_pool,
      ProducerF_&& producer_func,
      ConsumerF_&& consumer_func,
      Args&&... args
  ) {
    return make_scheduler_t<ProducerKind, ConsumerKind, ProducerInputRangeT, QueueT, ProducerF_, ConsumerF_, scheduler_traits<ProducerKind, ConsumerKind, ProducerInputRangeT, QueueT>>{
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
