#ifndef YK_STRING_HASH_HPP
#define YK_STRING_HASH_HPP

#include <string>
#include <string_view>

namespace yk {

template <class CharT, class Traits = std::char_traits<CharT>>
struct basic_string_hash {
  using is_transparent = void;

  [[nodiscard]] constexpr std::size_t operator()(const CharT* s) const noexcept { return std::hash<std::basic_string_view<CharT, Traits>>{}(s); }

  [[nodiscard]] constexpr std::size_t operator()(std::basic_string_view<CharT, Traits> sv) const noexcept {
    return std::hash<std::basic_string_view<CharT, Traits>>{}(sv);
  }

  template <class Allocator = std::allocator<CharT>>
  [[nodiscard]] constexpr std::size_t operator()(const std::basic_string<CharT, Traits, Allocator>& str) const noexcept {
    return std::hash<std::basic_string_view<CharT, Traits>>{}(str);
  }
};

using string_hash = basic_string_hash<char>;
using wstring_hash = basic_string_hash<wchar_t>;

using u8string_hash = basic_string_hash<char8_t>;
using u16string_hash = basic_string_hash<char16_t>;
using u32string_hash = basic_string_hash<char32_t>;

}  // namespace yk

#endif  // YK_STRING_HASH_HPP
