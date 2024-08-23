﻿#ifndef YK_BITMASK_ENUM_HPP
#define YK_BITMASK_ENUM_HPP

#include "yk/util/to_underlying.hpp"

#if !defined(NDEBUG)
#include <bit>
#endif

#include <cstdint>
#include <format>
#include <iosfwd>
#include <ranges>
#include <string_view>
#include <type_traits>
#include <utility>

#include <cassert>

namespace yk {

template <class T>
struct bitmask_enabled : std::false_type {};

template <class T>
constexpr bool bitmask_enabled_v = bitmask_enabled<T>::value;

template <class T>
concept BitmaskEnabledEnum = std::is_enum_v<T> && bitmask_enabled_v<T>;

namespace bitmask_operators {

template <BitmaskEnabledEnum T>
[[nodiscard]] constexpr T operator~(T a) noexcept {
  return static_cast<T>(~::yk::to_underlying(a));
}

template <BitmaskEnabledEnum T>
[[nodiscard]] constexpr T operator|(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return static_cast<T>(::yk::to_underlying(a) | ::yk::to_underlying(b));
}

template <BitmaskEnabledEnum T>
constexpr T& operator|=(T& a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return a = a | b;
}

template <BitmaskEnabledEnum T>
[[nodiscard]] constexpr T operator&(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return static_cast<T>(::yk::to_underlying(a) & ::yk::to_underlying(b));
}

template <BitmaskEnabledEnum T>
constexpr T& operator&=(T& a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return a = a & b;
}

template <BitmaskEnabledEnum T>
[[nodiscard]] constexpr T operator^(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return static_cast<T>(::yk::to_underlying(a) ^ ::yk::to_underlying(b));
}

template <BitmaskEnabledEnum T>
constexpr T& operator^=(T& a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return a = a ^ b;
}

template <BitmaskEnabledEnum T>
std::ostream& operator<<(std::ostream& os, const T& val) {
  return os << to_string(val);
}

}  // namespace bitmask_operators

template <BitmaskEnabledEnum T>
[[nodiscard]] constexpr bool contains(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return (::yk::to_underlying(a) & ::yk::to_underlying(b)) == ::yk::to_underlying(b);
}

template <BitmaskEnabledEnum T>
[[nodiscard]] constexpr bool contains_single_bit(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  assert(std::has_single_bit(::yk::to_underlying(b)));
  return ::yk::to_underlying(a) & ::yk::to_underlying(b);
}

template <BitmaskEnabledEnum T>
[[nodiscard]] constexpr bool contains_any_bit(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return ::yk::to_underlying(a) & ::yk::to_underlying(b);
}

namespace detail {

template <class T>
concept has_min_bit = std::same_as<std::remove_const_t<decltype(bitmask_enabled<T>::min_bit)>, int>;

template <class T>
concept has_max_bit = std::same_as<std::remove_const_t<decltype(bitmask_enabled<T>::max_bit)>, int>;

}  // namespace detail

template <BitmaskEnabledEnum T>
[[nodiscard]] constexpr auto each_bit(T flags) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  static_assert(detail::has_max_bit<T>);

  if constexpr (detail::has_min_bit<T>) {
    return std::views::iota(bitmask_enabled<T>::min_bit, bitmask_enabled<T>::max_bit + 1)                                         //
           | std::views::filter([cat = ::yk::to_underlying(flags)](int i) constexpr noexcept -> bool { return (cat >> i) & 1; })  //
           | std::views::transform([](int i) constexpr noexcept { return static_cast<T>(1u << i); });

  } else {
    return std::views::iota(0, bitmask_enabled<T>::max_bit + 1)                                                                   //
           | std::views::filter([cat = ::yk::to_underlying(flags)](int i) constexpr noexcept -> bool { return (cat >> i) & 1; })  //
           | std::views::transform([](int i) constexpr noexcept { return static_cast<T>(1u << i); });
  }
}

template <BitmaskEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flag(std::basic_string_view<CharT> str) noexcept {
  return bitmask_enabled<T>::parse(str);
}

template <BitmaskEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flag(const std::basic_string<CharT>& str) noexcept {
  return bitmask_enabled<T>::parse(str);
}

template <BitmaskEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flags(std::basic_string_view<CharT> str, std::basic_string_view<CharT> delim) noexcept {
  using namespace yk::bitmask_operators;

  T res{};

  for (const auto& r : str | std::views::split(delim)) {
    const std::basic_string_view<CharT> part_str{r};

    const auto part = parse_flag<T>(part_str);
    if (part == T{}) {
      return T{};
    }
    res |= part;
  }

  return res;
}

template <BitmaskEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flags(const std::basic_string<CharT>& str, std::basic_string_view<CharT> delim) noexcept {
  return parse_flags<T>(std::basic_string_view<CharT>{str}, delim);
}

template <BitmaskEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flags(std::basic_string_view<CharT> str, const std::basic_string<CharT>& delim) noexcept {
  return parse_flags<T>(str, std::basic_string_view<CharT>{delim});
}

template <BitmaskEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flags(const std::basic_string<CharT>& str, const std::basic_string<CharT>& delim) noexcept {
  return parse_flags<T>(std::basic_string_view<CharT>{str}, std::basic_string_view<CharT>{delim});
}

}  // namespace yk

namespace std {

template <yk::BitmaskEnabledEnum T, class CharT>
struct formatter<T, CharT> : formatter<std::underlying_type_t<T>, CharT> {
  using base_formatter = formatter<std::underlying_type_t<T>, CharT>;

  template <class Context>
  constexpr auto parse(Context& ctx) {
    if (ctx.begin() == ctx.end()) return ctx.end();
    has_format_spec_ = true;
    return base_formatter::parse(ctx);
  }

  template <class Context>
  auto format(const T& val, Context& ctx) const {
    if (has_format_spec_) {
      return base_formatter::format(::yk::to_underlying(val), ctx);
    } else {
      return format_to(ctx.out(), "{}", to_string(val));
    }
  }

private:
  bool has_format_spec_ = false;
};

}  // namespace std

#endif  // YK_BITMASK_ENUM_HPP