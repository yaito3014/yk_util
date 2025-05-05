#ifndef YK_VARIANT_BOOST_TRAITS_HPP
#define YK_VARIANT_BOOST_TRAITS_HPP

#include <boost/mpl/fold.hpp>
#include <boost/mpl/placeholders.hpp>

namespace yk::detail {

template <class... Ts>
struct type_list {};

template <class T, class... Ts>
struct type_list<T, Ts...> {
  using head = T;
};

template <class T, class R>
struct to_type_list;

template <class... Ts, class X>
struct to_type_list<type_list<Ts...>, X> {
  using type = type_list<Ts..., X>;
};

template <class BoostVariant>
using boost_variant_types_t = typename boost::mpl::fold<typename BoostVariant::types, type_list<>, to_type_list<boost::mpl::_1, boost::mpl::_2>>::type;

}  // namespace yk::detail

#endif  // YK_VARIANT_BOOST_TRAITS_HPP
