#ifndef YK_VARIANT_VIEW_STD_HASH_HPP
#define YK_VARIANT_VIEW_STD_HASH_HPP

#include "yk/variant_view.hpp"

#include <functional>
#include <type_traits>

#include <cstddef>

namespace std {

template <class Variant, class... Ts>
struct hash<yk::variant_view<Variant, Ts...>> : hash<std::remove_const_t<Variant>> {
  using is_transparent = void;
  std::size_t operator()(const yk::variant_view<Variant, Ts...>& view) const { return hash<std::remove_const_t<Variant>>::operator()(view.base()); }
};

}  // namespace std

#endif  // YK_VARIANT_VIEW_STD_HASH_HPP
