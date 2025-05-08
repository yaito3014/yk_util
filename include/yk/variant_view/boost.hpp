#ifndef YK_VARIANT_VIEW_BOOST_HPP
#define YK_VARIANT_VIEW_BOOST_HPP

#include "yk/util/exactly_once.hpp"
#include "yk/util/find_type_index.hpp"

#include "yk/variant/boost.hpp"
#include "yk/variant/boost/compare.hpp"
#include "yk/variant/boost/traits.hpp"
#include "yk/variant/traits.hpp"

#include "yk/variant_view/traits.hpp"
#include "yk/variant_view/variant_view.hpp"

#include <boost/mpl/contains.hpp>
#include <boost/variant/variant.hpp>

#include <compare>
#include <type_traits>
#include <utility>

#include <cstddef>

namespace yk {

template <class... Ts, class... Us, class T>
struct is_in_variant_view<variant_view<boost::variant<Ts...>, Us...>, T> : std::disjunction<std::is_same<Us, T>...> {
  static_assert(boost::mpl::contains<typename boost::variant<Ts...>::types, T>::type::value, "T must be in variant's template parameters");
};

template <class... Ts>
struct make_variant_view_result<boost::variant<Ts...>> {
  template <class TypeList>
  struct helper {};

  template <class... Us>
  struct helper<type_list<Us...>> {
    using type = variant_view<boost::variant<Ts...>, Us...>;
  };

  using type = typename helper<detail::boost_variant_types_t<boost::variant<Ts...>>>::type;
};

template <class T, class... Ts, class... Us>
[[nodiscard]] /* constexpr */ bool holds_alternative(const variant_view<boost::variant<Ts...>, Us...>& v) noexcept {
  return !v.invalid() && [&]<class... Vs>(type_list<Vs...>) {
    static_assert(core::exactly_once_v<T, Vs...>);
    return core::find_type_index_v<T, Vs...> == v.base().which();
  }(detail::boost_variant_types_t<boost::variant<Ts...>>{});
}

template <class T, class... Ts, class... Us>
[[nodiscard]] /* constexpr */ bool holds_alternative(const variant_view<const boost::variant<Ts...>, Us...>& v) noexcept {
  return !v.invalid() && [&]<class... Vs>(type_list<Vs...>) {
    static_assert(core::exactly_once_v<T, Vs...>);
    return core::find_type_index_v<T, Vs...> == v.base().which();
  }(detail::boost_variant_types_t<boost::variant<Ts...>>{});
}

// template <class... Ts>
// variant_view(const boost::variant<Ts...>&) -> /* impossible due to boost::recursive_variant_ */;

// template <class... Ts>
// variant_view(boost::variant<Ts...>&) -> /* impossible due to boost::recursive_variant_ */;

}  // namespace yk

#endif  // YK_VARIANT_VIEW_BOOST_HPP
