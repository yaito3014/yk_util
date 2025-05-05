#ifndef YK_VARIANT_VIEW_TRAITS_HPP
#define YK_VARIANT_VIEW_TRAITS_HPP

#include <type_traits>

namespace yk {

template <class VariantView, class T>
struct is_in_variant_view : std::false_type {};

template <class VariantView, class T>
inline constexpr bool is_in_variant_view_v = is_in_variant_view<VariantView, T>::value;

template <class Variant, class... Ts>
class variant_view;

template <class Variant, class... Ts>
struct make_variant_view_result {
  using type = variant_view<Variant, Ts...>;
};

template <class Variant, class... Ts>
using make_variant_view_result_t = typename make_variant_view_result<Variant, Ts...>::type;

}  // namespace yk

#endif  // YK_VARIANT_VIEW_TRAITS_HPP
