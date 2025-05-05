#ifndef YK_EXEC_SCHEDULER_DELTA_STATS_HPP
#define YK_EXEC_SCHEDULER_DELTA_STATS_HPP

#include "yk/exec/debug.hpp"
#include "yk/exec/scheduler_stats.hpp"

#include "yk/chrono.hpp"

#include <limits>


namespace yk::exec {

// scheduler_delta_stats is essentially: duration + scheduler_stats
struct scheduler_delta_stats
{
  using clock_type = std::chrono::steady_clock;
  using delta_type = clock_type::duration;

  scheduler_delta_stats() noexcept = default;

  scheduler_delta_stats(delta_type delta, const scheduler_stats& prev_stats, const scheduler_stats& stats) noexcept
    : scheduler_delta_stats(
      yk::duration_cast<double>(delta),
      stats.producer_output - prev_stats.producer_output,
      stats.consumer_input_processed - prev_stats.consumer_input_processed

#if YK_EXEC_DEBUG
      ,
      (stats.producer_time - prev_stats.producer_time) + (stats.consumer_time - prev_stats.consumer_time),
      stats.queue_overhead - prev_stats.queue_overhead
#endif
    )
  {}

  [[nodiscard]] double producer_output_per_sec() const noexcept { return producer_output_per_sec_; }
  [[nodiscard]] double consumer_process_per_sec() const noexcept { return consumer_process_per_sec_; }
  [[nodiscard]] double throughput_delta_per_sec() const noexcept { return throughput_delta_per_sec_; }

  [[nodiscard]] std::chrono::nanoseconds process_time() const noexcept
  {
#if YK_EXEC_DEBUG
    return process_time_;
#else
    return {};
#endif
  }

  [[nodiscard]] double queue_overhead() const noexcept
  {
#if YK_EXEC_DEBUG
    return queue_overhead_;
#else
    return std::numeric_limits<double>::quiet_NaN();
#endif
  }

private:
  scheduler_delta_stats(
      std::chrono::duration<double> delta_sec,
      long long producer_output_delta,
      long long consumer_process_delta

#if YK_EXEC_DEBUG
      ,
      std::chrono::nanoseconds process_time,
      std::chrono::nanoseconds queue_overhead
#endif
  ) noexcept
    : producer_output_per_sec_(producer_output_delta / delta_sec.count())
    , consumer_process_per_sec_(consumer_process_delta / delta_sec.count())
    , throughput_delta_per_sec_((producer_output_delta - consumer_process_delta) / delta_sec.count())

#if YK_EXEC_DEBUG
    , process_time_(process_time)
    , queue_overhead_(yk::duration_cast<double, std::nano>(queue_overhead) / process_time_)
#endif
  {
  }

  double producer_output_per_sec_ = 0, consumer_process_per_sec_ = 0, throughput_delta_per_sec_ = 0;

#if YK_EXEC_DEBUG
  std::chrono::nanoseconds process_time_{};
  double queue_overhead_ = 0;
#endif
};

} // yk::exec

#endif
