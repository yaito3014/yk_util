#ifndef YK_ENUM_HPP
#define YK_ENUM_HPP

#include <boost/assert.hpp>

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

namespace yk {

template <class T>
struct flag_enabled : std::false_type {};

template <class T>
constexpr bool flag_enabled_v = flag_enabled<T>::value;

template <class T>
concept FlagEnabledEnum = std::is_enum_v<T> && flag_enabled_v<T>;

namespace enum_operators {

template <FlagEnabledEnum T>
[[nodiscard]] constexpr T operator~(T a) noexcept {
  return static_cast<T>(~std::to_underlying(a));
}

template <FlagEnabledEnum T>
[[nodiscard]] constexpr T operator|(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return static_cast<T>(std::to_underlying(a) | std::to_underlying(b));
}

template <FlagEnabledEnum T>
constexpr T& operator|=(T& a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return a = a | b;
}

template <FlagEnabledEnum T>
[[nodiscard]] constexpr T operator&(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return static_cast<T>(std::to_underlying(a) & std::to_underlying(b));
}

template <FlagEnabledEnum T>
constexpr T& operator&=(T& a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return a = a & b;
}

template <FlagEnabledEnum T>
std::ostream& operator<<(std::ostream& os, T const& val) {
  return os << to_string(val);
}

}  // namespace enum_operators

template <FlagEnabledEnum T>
[[nodiscard]] constexpr bool contains(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return (std::to_underlying(a) & std::to_underlying(b)) == std::to_underlying(b);
}

template <FlagEnabledEnum T>
[[nodiscard]] constexpr bool contains_single_bit(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  BOOST_ASSERT(std::has_single_bit(std::to_underlying(b)));
  return std::to_underlying(a) & std::to_underlying(b);
}

template <FlagEnabledEnum T>
[[nodiscard]] constexpr bool contains_any_bit(T a, T b) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  return std::to_underlying(a) & std::to_underlying(b);
}

namespace detail {

template <class T>
concept has_min_bit = std::same_as<std::remove_const_t<decltype(flag_enabled<T>::min_bit)>, int>;

template <class T>
concept has_max_bit = std::same_as<std::remove_const_t<decltype(flag_enabled<T>::max_bit)>, int>;

}  // namespace detail

template <FlagEnabledEnum T>
[[nodiscard]] constexpr auto each_bit(T flags) noexcept {
  static_assert(std::is_unsigned_v<std::underlying_type_t<T>>);
  static_assert(detail::has_max_bit<T>);

  if constexpr (detail::has_min_bit<T>) {
    return std::views::iota(flag_enabled<T>::min_bit, flag_enabled<T>::max_bit + 1) |
           std::views::filter([cat = std::to_underlying(flags)](int i) constexpr noexcept -> bool { return (cat >> i) & 1; }) |
           std::views::transform([](int i) constexpr noexcept { return static_cast<T>(1u << i); });

  } else {
    return std::views::iota(0, flag_enabled<T>::max_bit + 1) |
           std::views::filter([cat = std::to_underlying(flags)](int i) constexpr noexcept -> bool { return (cat >> i) & 1; }) |
           std::views::transform([](int i) constexpr noexcept { return static_cast<T>(1u << i); });
  }
}

template <FlagEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flag(std::basic_string_view<CharT> str) noexcept {
  return flag_enabled<T>::parse(str);
}

template <FlagEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flag(std::basic_string<CharT> const& str) noexcept {
  return flag_enabled<T>::parse(str);
}

template <FlagEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flags(std::basic_string_view<CharT> str, std::basic_string_view<CharT> delim) noexcept {
  using namespace yk::enum_operators;

  T res{};

  for (auto const& r : str | std::views::split(delim)) {
    std::basic_string_view<CharT> const part_str{r};

    auto const part = parse_flag<T>(part_str);
    if (part == T{}) {
      return T{};
    }
    res |= part;
  }

  return res;
}

template <FlagEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flags(std::basic_string<CharT> const& str, std::basic_string_view<CharT> delim) noexcept {
  return parse_flags<T>(std::basic_string_view<CharT>{str}, delim);
}

template <FlagEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flags(std::basic_string_view<CharT> str, std::basic_string<CharT> const& delim) noexcept {
  return parse_flags<T>(str, std::basic_string_view<CharT>{delim});
}

template <FlagEnabledEnum T, class CharT>
[[nodiscard]] constexpr T parse_flags(std::basic_string<CharT> const& str, std::basic_string<CharT> const& delim) noexcept {
  return parse_flags<T>(std::basic_string_view<CharT>{str}, std::basic_string_view<CharT>{delim});
}

}  // namespace yk

namespace std {

template <yk::FlagEnabledEnum T, class CharT>
struct formatter<T, CharT> : formatter<std::underlying_type_t<T>, CharT> {
  using base_formatter = formatter<std::underlying_type_t<T>, CharT>;

  template <class Context>
  constexpr auto parse(Context& ctx) {
    if (ctx.begin() == ctx.end()) return ctx.end();
    has_format_spec_ = true;
    return base_formatter::parse(ctx);
  }

  template <class Context>
  auto format(T const& val, Context& ctx) const {
    if (has_format_spec_) {
      return base_formatter::format(std::to_underlying(val), ctx);

    } else {
      return format_to(ctx.out(), "{}", to_string(val));
    }
  }

private:
  bool has_format_spec_ = false;
};

}  // namespace std

#endif  // YK_ENUM_HPP
