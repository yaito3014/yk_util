#ifndef YK_EXEC_SCHEDULER_STATS_TRACKER_HPP
#define YK_EXEC_SCHEDULER_STATS_TRACKER_HPP

#include "yk/exec/debug.hpp"// for ODR violation safety
#include "yk/exec/scheduler_stats.hpp"

#include <chrono>
#include <functional>
#include <concepts>

namespace yk::exec {

class scheduler_stats_tracker {
public:
  using callback_type = std::move_only_function<void(const scheduler_stats_tracker&)>;
  using clock_type = std::chrono::steady_clock;

  scheduler_stats_tracker() noexcept = default;

  explicit scheduler_stats_tracker(const std::chrono::milliseconds& interval) noexcept
    : interval_(interval)
  {}

  template <std::invocable<const scheduler_stats_tracker&> F>
  scheduler_stats_tracker(const std::chrono::milliseconds& interval, F&& f)
    : interval_(interval)
    , callback_(std::forward<F>(f))
  {}

  template <std::invocable<const scheduler_stats_tracker&> F>
  scheduler_stats_tracker(F&& f)
    : callback_(std::forward<F>(f))
  {}

  [[nodiscard]] std::chrono::milliseconds get_interval() const noexcept { return interval_; }
  void set_interval(std::chrono::milliseconds interval) noexcept { interval_ = interval; }

  template <std::invocable<const scheduler_stats_tracker&> F>
  void set_callback(F&& f)
  {
    callback_ = std::forward<F>(f);
  }

  [[nodiscard]] const callback_type& get_callback() const noexcept { return callback_; }

  // ------------------------------------------------------

  [[nodiscard]] clock_type::time_point tick() const noexcept { return tick_; }
  [[nodiscard]] clock_type::time_point last_tick() const noexcept { return last_tick_; }

  [[nodiscard]] clock_type::time_point first_tick() const noexcept { return first_tick_; }
  void reset_first_tick() noexcept { first_tick_ = clock_type::now(); }

  template <class Duration = std::chrono::duration<double>>
  [[nodiscard]]
  Duration total_time() const noexcept { return std::chrono::duration_cast<Duration>(tick_ - first_tick_); }

  template <class Duration = std::chrono::duration<double>>
  [[nodiscard]]
  Duration delta_time() const noexcept { return std::chrono::duration_cast<Duration>(tick_ - last_tick_); }

  [[nodiscard]]
  const scheduler_stats& stats() const noexcept { return stats_; }

  [[nodiscard]]
  const scheduler_stats& last_stats() const noexcept { return last_stats_; }

  // ------------------------------------------------------

  [[nodiscard]]
  bool interval_elapsed() const noexcept
  {
    return clock_type::now() - last_tick_ >= interval_;
  }

  void tick(const scheduler_stats& stats)
  {
    tick_ = clock_type::now();
    stats_ = stats;

    if (callback_ && stats_.count_updated(last_stats_)) {
      callback_(*this);
    }

    last_tick_ = tick_;
    last_stats_ = stats_;
  }

private:
  std::chrono::milliseconds interval_{1000};
  callback_type callback_;
  clock_type::time_point first_tick_{}, tick_{}, last_tick_{};

  scheduler_stats stats_{}, last_stats_{};
};

} // yk::exec

#endif
