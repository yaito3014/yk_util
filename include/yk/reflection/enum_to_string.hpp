#ifndef YK_REFLECTION_ENUM_TO_STRING_HPP
#define YK_REFLECTION_ENUM_TO_STRING_HPP

#include <version>

#if __cpp_lib_reflection >= 202500L && __cpp_lib_define_static >= 202500L

#include <meta>
#include <optional>
#include <string_view>
#include <type_traits>

namespace yk {

template <class E>
  requires std::is_enum_v<E>
[[nodiscard]] constexpr std::optional<std::string_view> enum_to_identifier(E value) noexcept
{
  if constexpr (std::meta::is_enumerable_type(^^E))
    template for (constexpr std::meta::info enumerator : std::define_static_array(std::meta::enumerators_of(^^E))) {
      if ([:enumerator:] == value) {
        return std::meta::identifier_of(enumerator);
      }
    }
  return std::nullopt;
}

}  // namespace yk

#endif

#endif  // YK_REFLECTION_ENUM_TO_STRING_HPP
