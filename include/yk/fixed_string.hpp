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

  using traits_type = Traits;
  using value_type = CharT;
  using iterator = typename storage_type::iterator;
  using const_iterator = typename storage_type::const_iterator;
  using string_view_type = std::basic_string_view<CharT, Traits>;

  constexpr basic_fixed_string() noexcept = default;
  constexpr basic_fixed_string(const CharT (&str)[N + 1]) { std::ranges::copy(str, data.begin()); }

  [[nodiscard]] constexpr iterator begin() noexcept { return data.begin(); }
  [[nodiscard]] constexpr const_iterator begin() const noexcept { return data.begin(); }
  [[nodiscard]] constexpr iterator end() noexcept { return data.end() - 1; }
  [[nodiscard]] constexpr const_iterator end() const noexcept { return data.end() - 1; }

  [[nodiscard]] constexpr operator string_view_type() const { return {begin(), end()}; }
};

template <class CharT, std::size_t N>
basic_fixed_string(const CharT (&)[N]) -> basic_fixed_string<CharT, N - 1>;

template <std::size_t N>
using fixed_string = basic_fixed_string<char, N>;

}  // namespace yk

#endif  // YK_FIXED_STRING_HPP
