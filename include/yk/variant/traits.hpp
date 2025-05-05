#ifndef YK_VARIANT_TRAITS_HPP
#define YK_VARIANT_TRAITS_HPP

#include "yk/util/always_false.hpp"

#include <type_traits>

namespace yk {

template <class Variant, class T>
struct is_in_variant : std::false_type {};

template <class Variant, class T>
inline constexpr bool is_in_variant_v = is_in_variant<Variant, T>::value;

template <class Variant>
struct variant_dispatch {
  static_assert(core::always_false_v<Variant>, "variant_dispatch is not specialized for this variant");
};

template <class Variant>
struct is_variant_like : std::false_type {};

template <class Variant>
inline constexpr bool is_variant_like_v = is_variant_like<Variant>::value;

template <class Variant>
concept variant_like = is_variant_like_v<Variant>;

}  // namespace yk

#endif  // YK_VARIANT_TRAITS_HPP
