#ifndef YK_EXEC_SCHEDULER_STATS_HPP
#define YK_EXEC_SCHEDULER_STATS_HPP

#include "yk/exec/debug.hpp"

#include <chrono>
#include <compare>
#include <format>
#include <functional>
#include <stdexcept>

namespace yk::exec {

struct scheduler_stats {
  using count_type = long long;

  count_type
#if YK_EXEC_DEBUG
      producer_input_consumed = 0,
#endif

      producer_input_processed = 0,

      // different domain ---------------------

      producer_output = 0,

#if YK_EXEC_DEBUG
      consumer_input_consumed = 0,
#endif

      consumer_input_processed = 0;

  [[nodiscard]]
  constexpr bool operator==(const scheduler_stats&) const noexcept = default;

  [[nodiscard]]
  constexpr std::strong_ordering operator<=>(const scheduler_stats&) const noexcept = default;

  void validate_counter_consistency(const count_type producer_input_total) const {
#if YK_EXEC_DEBUG
    if (producer_input_consumed > producer_input_total) {
      throw std::logic_error{std::format("producer overconsumption (input total: {}, consumed: {})", producer_input_total, producer_input_consumed)};
    }

    if (producer_input_processed > producer_input_consumed) {
      throw std::logic_error{std::format("producer overprocess (consumed: {}, processed: {})", producer_input_consumed, producer_input_processed)};
    }
#endif

    if (producer_input_processed > producer_input_total) {
      throw std::logic_error{std::format("producer overprocess (input total: {}, processed: {})", producer_input_total, producer_input_processed)};
    }

#if YK_EXEC_DEBUG
    if (consumer_input_consumed > producer_output) {
      throw std::logic_error{std::format("consumer overconsumption (producer output: {}, consumed: {})", producer_output, consumer_input_consumed)};
    }

    if (consumer_input_processed > consumer_input_consumed) {
      throw std::logic_error{std::format("consumer overprocess (consumed: {}, processed: {})", consumer_input_consumed, consumer_input_processed)};
    }
#endif
    if (consumer_input_processed > producer_output) {
      throw std::logic_error{std::format("consumer overprocess (producer output: {}, consumer processed: {})", producer_output, consumer_input_processed)};
    }
  }
};

class scheduler_stats_tracker {
public:
  using callback_type = std::move_only_function<void(const scheduler_stats_tracker&)>;
  using clock_type = std::chrono::steady_clock;

  scheduler_stats_tracker() noexcept = default;

  explicit scheduler_stats_tracker(const std::chrono::milliseconds& interval) noexcept  //
      : interval_(interval) {}

  template <std::invocable<const scheduler_stats_tracker&> F>
  scheduler_stats_tracker(const std::chrono::milliseconds& interval, F&& f)  //
      : interval_(interval), callback_(std::forward<F>(f)) {}

  template <std::invocable<const scheduler_stats_tracker&> F>
  scheduler_stats_tracker(F&& f)  //
      : callback_(std::forward<F>(f)) {}

  [[nodiscard]]
  std::chrono::milliseconds get_interval() const noexcept {
    return interval_;
  }

  void set_interval(std::chrono::milliseconds interval) noexcept {  //
    interval_ = interval;
  }

  template <std::invocable<const scheduler_stats_tracker&> F>
  void set_callback(F&& f) {
    callback_ = std::forward<F>(f);
  }

  [[nodiscard]]
  const callback_type& get_callback() const noexcept {
    return callback_;
  }

  // ------------------------------------------------------

  [[nodiscard]]
  clock_type::time_point tick() const noexcept {
    return tick_;
  }

  [[nodiscard]]
  clock_type::time_point last_tick() const noexcept {
    return last_tick_;
  }

  [[nodiscard]]
  const scheduler_stats& stats() const noexcept {
    return stats_;
  }

  [[nodiscard]]
  const scheduler_stats& last_stats() const noexcept {
    return last_stats_;
  }

  // ------------------------------------------------------

  [[nodiscard]]
  bool interval_elapsed() const noexcept {
    return clock_type::now() - last_tick_ >= interval_;
  }

  void tick(const scheduler_stats& stats) {
    tick_ = clock_type::now();
    stats_ = stats;

    if (callback_) {
      callback_(*this);
    }

    last_tick_ = tick_;
    last_stats_ = stats_;
  }

private:
  std::chrono::milliseconds interval_{1000};
  callback_type callback_;
  clock_type::time_point tick_{}, last_tick_{};

  scheduler_stats stats_{}, last_stats_{};
};

}  // namespace yk::exec

#endif
