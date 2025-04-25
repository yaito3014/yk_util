#ifndef YK_EXEC_SCHEDULER_STATS_HPP
#define YK_EXEC_SCHEDULER_STATS_HPP

#include "yk/exec/debug.hpp"
#include "yk/exec/scheduler_traits.hpp"

#include <limits>
#include <chrono>
#include <compare>
#include <format>
#include <functional>
#include <stdexcept>

namespace yk::exec {

struct scheduler_stats {
  using count_type = long long;

  bool is_running = false;

  // task count is unpredictable; possibly because the ProducerInputRangeT was not sized_range.
  static constexpr count_type UNPREDICTABLE = static_cast<count_type>(-1);
  count_type producer_input_total = UNPREDICTABLE; // type is producer_input_value_type

  count_type producer_input_consumed = 0; // type is producer_input_value_type
  count_type producer_input_processed = 0; // type is producer_input_value_type

  // ----- different domain -----

  count_type producer_output = 0; // type is T

#if YK_EXEC_DEBUG
  count_type consumer_input_consumed = 0; // type is T
#endif

  count_type consumer_input_processed = 0; // type is T

  // =============================================

  scheduler_stats() noexcept = default;

  template<ProducerInputRange ProducerInputRangeT> requires std::ranges::sized_range<ProducerInputRangeT>
  explicit scheduler_stats(ProducerInputRangeT const& producer_inputs)
  {
    auto const raw_size = std::ranges::size(producer_inputs);
    using raw_size_t = decltype(raw_size);

    if (raw_size > static_cast<raw_size_t>(std::numeric_limits<count_type>::max())) {
      throw std::invalid_argument{"too many producer inputs"};
    }

    producer_input_total = static_cast<count_type>(raw_size);
  }

  template<ProducerInputRange ProducerInputRangeT> requires (!std::ranges::sized_range<ProducerInputRangeT>)
  explicit scheduler_stats(ProducerInputRangeT const&)
  {}

  [[nodiscard]]
  constexpr bool operator==(const scheduler_stats&) const noexcept = default;

  [[nodiscard]]
  constexpr std::strong_ordering operator<=>(const scheduler_stats&) const noexcept = default;

  // =============================================

  [[nodiscard]]
  bool is_producer_input_processed_all() const noexcept
  {
    return producer_input_processed_all_;
  }

  void set_producer_input_processed_all()
  {
    if (producer_input_processed_all_) {
      throw std::logic_error{"set_producer_input_processed_all has been called multiple times; this leads to invalid (duplicate) wakeups"};
    }

    if (producer_input_total == UNPREDICTABLE) {
      producer_input_processed_all_ = true;

    } else {
      producer_input_processed_all_ = true;
      if (producer_input_processed != producer_input_total) {
        throw std::logic_error{std::format("attempted to set producer_input_processed_all, but total count ({}) and processed count ({}) does not match", producer_input_total, producer_input_processed)};
      }
    }
  }

  void validate_counter_consistency(const count_type producer_input_total) const
  {
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

    if (producer_input_total != UNPREDICTABLE) {
      if (producer_input_processed_all_) {
        if (producer_input_processed != producer_input_total) {
          throw std::logic_error{std::format("producer input has been completed, but processed count ({}) does not match the total input ({})", producer_input_processed, producer_input_total)};
        }

      } else {
        if (producer_input_processed == producer_input_total) {
          throw std::logic_error{std::format("producer input was marked as NOT completed, but processed count already fulfills the total input ({})", producer_input_processed)};
        }
      }
    }
  }

private:
  bool producer_input_processed_all_ = false;
};

}  // namespace yk::exec

#endif
