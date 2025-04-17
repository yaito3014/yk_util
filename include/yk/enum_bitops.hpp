#ifndef YK_ENUM_BITOPS_HPP
#define YK_ENUM_BITOPS_HPP

#include "yk/util/to_underlying.hpp"

#if !defined(NDEBUG)
#include <bit>
#endif

#include <concepts>
#include <type_traits>
#include <utility>

#include <cassert>
#include <cstdint>

namespace yk {

template <class T>
struct bitops_enabled : std::false_type {};

template <class T>
constexpr bool bitops_enabled_v = bitops_enabled<T>::value;

namespace detail {

template <class T>
concept bitops_enum_has_min_bit = std::same_as<std::remove_const_t<decltype(bitops_enabled<T>::min_bit)>, int>;

template <class T>
concept bitops_enum_has_max_bit = std::same_as<std::remove_const_t<decltype(bitops_enabled<T>::max_bit)>, int>;

}  // namespace detail

template <class T>
concept BitopsEnabledEnum = std::is_enum_v<T> && bitops_enabled_v<T>;

inline namespace bitops_operators {

template <BitopsEnabledEnum T>
[[nodiscard]] constexpr T operator~(T a) noexcept {
  return static_cast<T>(~::yk::to_underlying(a));
}

template <BitopsEnabledEnum T>
[[nodiscard]] constexpr T operator|(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return static_cast<T>(::yk::to_underlying(a) | ::yk::to_underlying(b));
}

template <BitopsEnabledEnum T>
constexpr T& operator|=(T& a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return a = a | b;
}

template <BitopsEnabledEnum T>
[[nodiscard]] constexpr T operator&(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return static_cast<T>(::yk::to_underlying(a) & ::yk::to_underlying(b));
}

template <BitopsEnabledEnum T>
constexpr T& operator&=(T& a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return a = a & b;
}

template <BitopsEnabledEnum T>
[[nodiscard]] constexpr T operator^(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return static_cast<T>(::yk::to_underlying(a) ^ ::yk::to_underlying(b));
}

template <BitopsEnabledEnum T>
constexpr T& operator^=(T& a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return a = a ^ b;
}

}  // namespace bitops_operators

template <BitopsEnabledEnum T>
[[nodiscard]] constexpr bool contains(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return (::yk::to_underlying(a) & ::yk::to_underlying(b)) == ::yk::to_underlying(b);
}

template <BitopsEnabledEnum T>
[[nodiscard]] constexpr bool contains_single_bit(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  assert(std::has_single_bit(::yk::to_underlying(b)));
  return ::yk::to_underlying(a) & ::yk::to_underlying(b);
}

template <BitopsEnabledEnum T>
[[nodiscard]] constexpr bool contains_any_bit(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return ::yk::to_underlying(a) & ::yk::to_underlying(b);
}

}  // namespace yk

#endif  // YK_BITOPS_ENUM_HPP
