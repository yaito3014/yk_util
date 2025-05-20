#ifndef YK_FIXED_STRING_HPP
#define YK_FIXED_STRING_HPP

#include <algorithm>
#include <array>
#include <string>
#include <string_view>

#include <cstddef>

namespace yk {

template <class CharT, std::size_t N, class Traits = std::char_traits<CharT>>
struct basic_fixed_string {
  using storage_type = std::array<CharT, N + 1>;

  storage_type data{};

  constexpr basic_fixed_string() noexcept = default;
  constexpr basic_fixed_string(const CharT (&str)[N + 1]) { std::ranges::copy(str, data.begin()); }
};

template <class CharT, std::size_t N>
basic_fixed_string(const CharT (&)[N]) -> basic_fixed_string<CharT, N - 1>;

template <std::size_t N>
using fixed_string = basic_fixed_string<char, N>;

namespace fixed_string_literals {

template <basic_fixed_string Str>
constexpr auto operator""_fixed()
{
  return Str;
}

}  // namespace fixed_string_literals

}  // namespace yk

#endif  // YK_FIXED_STRING_HPP
