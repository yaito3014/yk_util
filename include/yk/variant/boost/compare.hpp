#ifndef YK_VARIANT_BOOST_COMPARE_HPP
#define YK_VARIANT_BOOST_COMPARE_HPP

#include "yk/util/always_false.hpp"
#include "yk/variant/boost/traits.hpp"

#include <boost/variant/variant.hpp>

#include <compare>
#include <type_traits>
#include <utility>

namespace yk {

namespace detail {

template <class TypeList>
struct calc_category;

template <class... Ts>
struct calc_category<type_list<Ts...>> {
  using type = std::common_comparison_category_t<std::compare_three_way_result_t<Ts>...>;
};

template <class Variant>
struct compare_impl;

template <class... Ts>
struct compare_impl<boost::variant<Ts...>> {
  using category_t = typename calc_category<detail::boost_variant_types_t<boost::variant<Ts...>>>::type;

  [[nodiscard]] static constexpr category_t apply(const boost::variant<Ts...>& lhs, const boost::variant<Ts...>& rhs) {
    if (lhs.which() != rhs.which()) return lhs.which() <=> rhs.which();

    const auto vis = [](const auto& a, const auto& b) -> category_t {
      if constexpr (std::same_as<decltype(a), decltype(b)>) {
        if constexpr (std::same_as<category_t, std::strong_ordering>) {
          return std::compare_strong_order_fallback(a, b);
        } else if constexpr (std::same_as<category_t, std::weak_ordering>) {
          return std::compare_weak_order_fallback(a, b);
        } else if constexpr (std::same_as<category_t, std::partial_ordering>) {
          return std::compare_partial_order_fallback(a, b);
        } else {
          static_assert(core::always_false_v<category_t>, "unrecognized ordering");
        }
      } else {
        return category_t::equivalent;  // unreachable
      }
    };
    return boost::apply_visitor(vis, lhs, rhs);
  }
};

}  // namespace detail

struct compare_three_way {
  template <class T, class U>
  constexpr auto operator()(const T& x, const U& y) const {
    return std::compare_three_way{}(x, y);
  }

  template <class... Ts>
  constexpr auto operator()(const boost::variant<Ts...>& lhs, const boost::variant<Ts...>& rhs) const {
    return detail::compare_impl<boost::variant<Ts...>>::apply(lhs, rhs);
  }
};

}  // namespace yk

#endif  // YK_VARIANT_BOOST_COMPARE_HPP
