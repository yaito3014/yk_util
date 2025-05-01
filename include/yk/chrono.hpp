#ifndef YK_CHRONO_HPP
#define YK_CHRONO_HPP

#include "yk/util/specialization_of.hpp"

#include <chrono>
#include <ratio>


namespace yk {

//
// Wrappers for <chrono> QoL issues, for instance:
//
//   // before
//   auto delta = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(part_time_ns) / total_time_ns;
//
//   // after
//   auto delta = yk::duration_cast<double, std::nano>(part_time_ns) / total_time_ns;
//
// Also, the original form is supported as a fallback:
//
//   auto delta = yk::duration_cast<some_duration_type>(part_time_ns) / total_time_ns;
//

template <class Rep, class Period = std::ratio<1>, class... Args> requires (!specialization_of<Rep, std::chrono::duration>)
[[nodiscard]] constexpr std::chrono::duration<Rep, Period>
duration_cast(const std::chrono::duration<Args...>& d)
  noexcept(noexcept(std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(d))) // strengthened
{
  return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(d);
}

// fallback
template <class Duration, class... Args> requires (specialization_of<Duration, std::chrono::duration>)
[[nodiscard]] constexpr Duration
duration_cast(const std::chrono::duration<Args...>& d)
  noexcept(noexcept(std::chrono::duration_cast<Duration>(d))) // strengthened
{
  return std::chrono::duration_cast<Duration>(d);
}

} // yk

#endif
