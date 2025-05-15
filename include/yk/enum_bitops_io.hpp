#ifndef YK_ENUM_BITOPS_IO_HPP
#define YK_ENUM_BITOPS_IO_HPP

#include "yk/detail/string_like.hpp"

#include "yk/enum_bitops.hpp"

#include <format>
#include <iosfwd>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>

namespace yk {

// Fallback stream outputter for arbitrary BitopsEnabledEnum.
// This overload is intended for the last candidate for the type,
// because the user-provided operator<< (if exists) should be preferred in most cases.
// If ADL apocalypse kicks in, then we must reconsider this implementation.

inline namespace bitops_operators {

template <BitopsEnabledEnum T>
std::ostream& operator<<(std::ostream& os, const T& val) {
  if constexpr (requires {
                  { to_string(val) } -> std::convertible_to<const std::string&>;
                }) {
    return os << to_string(val);
  } else {
    return os << std::to_underlying(val);
  }
}

}  // namespace bitops_operators

template <BitopsEnabledEnum T, detail::StringLike Str>
[[nodiscard]] constexpr T parse_flag(const Str& str) noexcept {
  return bitops_enabled<T>::parse(std::basic_string_view{str});
}

template <BitopsEnabledEnum T, detail::StringLike Str, detail::StringLike Delim>
[[nodiscard]] constexpr T parse_flags(const Str& str, const Delim& delim) noexcept {
  using namespace yk::bitops_operators;

  std::basic_string_view str_sv{str};
  std::basic_string_view delim_sv{delim};

  T res{};

  for (const auto& r : str_sv | std::views::split(delim_sv)) {
    const std::basic_string_view part_str{r.begin(), r.end()};

    const auto part = parse_flag<T>(part_str);
    if (part == T{}) {
      return T{};
    }
    res |= part;
  }

  return res;
}

}  // namespace yk

namespace std {

template <yk::BitopsEnabledEnum T, class CharT>
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
      return base_formatter::format(std::to_underlying(val), ctx);
    } else {
      return format_to(ctx.out(), "{}", to_string(val));
    }
  }

private:
  bool has_format_spec_ = false;
};

}  // namespace std

#endif
