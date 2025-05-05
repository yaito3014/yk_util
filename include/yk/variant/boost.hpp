#ifndef YK_VARIANT_BOOST_HPP
#define YK_VARIANT_BOOST_HPP

#include "yk/util/all_same.hpp"
#include "yk/util/exactly_once.hpp"
#include "yk/util/find_type_index.hpp"
#include "yk/util/forward_like.hpp"
#include "yk/util/pack_indexing.hpp"

#include "yk/variant/boost/traits.hpp"
#include "yk/variant/traits.hpp"
#include "yk/variant/variant.hpp"

#include <type_traits>
#include <variant>

#include <boost/mpl/contains.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/multivisitors.hpp>
#include <boost/variant/variant.hpp>

namespace yk {

template <class... Ts, class T>
struct is_in_variant<boost::variant<Ts...>, T> : std::bool_constant<boost::mpl::contains<typename boost::variant<Ts...>::types, T>::type::value> {};

template <class... Ts>
struct is_variant_like<boost::variant<Ts...>> : std::true_type {};

template <class... Ts>
struct variant_dispatch<boost::variant<Ts...>> {
private:
  template <class Visitor, class Res>
  struct Wrapper {
    using result_type = Res;
    Visitor vis;
    template <class T>
    constexpr Res operator()(T&& x) const {
      return std::invoke(std::forward<Visitor>(vis), std::forward<T>(x));
    }
    template <class T>
    constexpr Res operator()(T&& x) {
      return std::invoke(std::forward<Visitor>(vis), std::forward<T>(x));
    }
  };

public:
  template <class Visitor, class Variant>
  static constexpr decltype(auto) apply_visit(Visitor&& vis, Variant&& variant) {
    []<class... Us>(detail::type_list<Us...>) {
      static_assert(core::is_all_same_v<std::invoke_result_t<Visitor, forward_like_t<Variant, Us>>...>,
                    "visitor must return same type for all possible parameters");
    }(detail::boost_variant_types_t<std::remove_cvref_t<Variant>>{});
    return boost::apply_visitor(std::forward<Visitor>(vis), std::forward<Variant>(variant));
  }

  template <class Res, class Visitor, class Variant>
  static constexpr Res apply_visit(Visitor&& vis, Variant&& variant) {
    Wrapper<Visitor, Res> wrapper{std::forward<Visitor>(vis)};
    return boost::apply_visitor(wrapper, std::forward<Variant>(variant));
  }

  template <class T, class BoostVariant>
  static constexpr decltype(auto) apply_get(BoostVariant&& variant) try {
    return boost::get<T>(std::forward<BoostVariant>(variant));
  } catch (const boost::bad_get&) {
    throw std::bad_variant_access{};
  }

  template <std::size_t I, class BoostVariant>
  static constexpr decltype(auto) apply_get(BoostVariant&& variant) try {
    return [&]<class... Vs>(detail::type_list<Vs...>) -> decltype(auto) {
      return boost::get<pack_indexing_t<I, Vs...>>(std::forward<BoostVariant>(variant));
    }(detail::boost_variant_types_t<std::remove_cvref_t<BoostVariant>>{});
  } catch (const boost::bad_get&) {
    throw std::bad_variant_access{};
  }

  template <class T, class BoostVariant>
  static constexpr auto apply_get(BoostVariant* variant) noexcept {
    return boost::get<T>(variant);
  }

  template <std::size_t I, class BoostVariant>
  static constexpr auto apply_get(BoostVariant* variant) noexcept {
    return [&]<class... Vs>(detail::type_list<Vs...>) {
      return boost::get<pack_indexing_t<I, Vs...>>(variant);
    }(detail::boost_variant_types_t<std::remove_cvref_t<BoostVariant>>{});
  }

  static constexpr std::size_t apply_index(const boost::variant<Ts...>& var) noexcept { return static_cast<std::size_t>(var.which()); }
};

template <class T, class... Ts>
[[nodiscard]] /* constexpr */ bool holds_alternative(const boost::variant<Ts...>& v) noexcept {
  return [&]<class... Us>(detail::type_list<Us...>) {
    static_assert(core::exactly_once_v<T, Us...>);
    return core::find_type_index_v<T, Us...> == v.which();
  }(detail::boost_variant_types_t<boost::variant<Ts...>>{});
}

}  // namespace yk

#endif  // YK_VARIANT_BOOST_HPP
