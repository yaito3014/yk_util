#ifndef YK_VARIANT_VIEW_STD_HPP
#define YK_VARIANT_VIEW_STD_HPP

#include "yk/variant/std.hpp"
#include "yk/variant/traits.hpp"

#include "yk/variant_view/traits.hpp"
#include "yk/variant_view/variant_view.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>
#include <variant>

namespace yk {

template <class... Ts>
struct make_variant_view_result<std::variant<Ts...>> {
  using type = variant_view<std::variant<Ts...>, Ts...>;
};

template <class... Ts, class... Us, class T>
struct is_in_variant_view<variant_view<std::variant<Ts...>, Us...>, T> : std::disjunction<std::is_same<Us, T>...> {
  static_assert((... || std::is_same_v<Ts, T>), "T must be in variant's template parameters");
};

template <class T, class... Ts, class... Us>
[[nodiscard]] constexpr bool holds_alternative(const variant_view<std::variant<Ts...>, Us...>& v) noexcept {
  return !v.invalid() && std::holds_alternative<T>(v.base());
}

template <class T, class... Ts, class... Us>
[[nodiscard]] constexpr bool holds_alternative(const variant_view<const std::variant<Ts...>, Us...>& v) noexcept {
  return !v.invalid() && std::holds_alternative<T>(v.base());
}

template <class... Ts>
variant_view(const std::variant<Ts...>&) -> variant_view<const std::variant<Ts...>, Ts...>;

template <class... Ts>
variant_view(std::variant<Ts...>&) -> variant_view<std::variant<Ts...>, Ts...>;

}  // namespace yk

#endif  // YK_VARIANT_VIEW_STD_HPP
