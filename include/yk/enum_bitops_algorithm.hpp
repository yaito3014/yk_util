#ifndef YK_ENUM_BITOPS_ALGORITHM_HPP
#define YK_ENUM_BITOPS_ALGORITHM_HPP

#include "yk/util/to_underlying.hpp"
#include "yk/enum_bitops.hpp"

#include <limits>
#include <ranges>

namespace yk {

template <BitopsEnabledEnum T>
[[nodiscard]] constexpr auto each_bit(T flags) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  static_assert(detail::bitops_enum_has_max_bit<T>);
  static_assert(bitops_enabled<T>::max_bit < std::numeric_limits<std::underlying_type_t<T>>::digits);

  if constexpr (detail::bitops_enum_has_min_bit<T>) {
    static_assert(bitops_enabled<T>::min_bit <= bitops_enabled<T>::max_bit);
    return std::views::iota(bitops_enabled<T>::min_bit, bitops_enabled<T>::max_bit + 1)                                           //
           | std::views::filter([cat = ::yk::to_underlying(flags)](int i) constexpr noexcept -> bool { return (cat >> i) & 1; })  //
           | std::views::transform([](int i) constexpr noexcept { return static_cast<T>(static_cast<std::underlying_type_t<T>>(1) << i); });
  } else {
    return std::views::iota(0, bitops_enabled<T>::max_bit + 1)                                                                    //
           | std::views::filter([cat = ::yk::to_underlying(flags)](int i) constexpr noexcept -> bool { return (cat >> i) & 1; })  //
           | std::views::transform([](int i) constexpr noexcept { return static_cast<T>(static_cast<std::underlying_type_t<T>>(1) << i); });
  }
}

}  // namespace yk

#endif
