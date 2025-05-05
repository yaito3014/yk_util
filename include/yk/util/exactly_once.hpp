#ifndef YK_UTIL_EXACTLY_ONCE_HPP
#define YK_UTIL_EXACTLY_ONCE_HPP

#include <type_traits>

namespace yk::core {

namespace detail {

template <bool Found, class T, class... Us>
struct exactly_once_impl : std::bool_constant<Found> {};

template <class T, class U, class... Us>
struct exactly_once_impl<false, T, U, Us...> : exactly_once_impl<std::is_same_v<T, U>, T, Us...> {};

template <class T, class U, class... Us>
struct exactly_once_impl<true, T, U, Us...> : std::conditional_t<std::is_same_v<T, U>, std::false_type, exactly_once_impl<true, T, Us...>> {};

}  // namespace detail

template <class T, class... Ts>
struct exactly_once : detail::exactly_once_impl<false, T, Ts...> {
  static_assert(sizeof...(Ts) > 0, "cannot determine existence of T in empty sets");
};

template <class T, class... Ts>
inline constexpr bool exactly_once_v = exactly_once<T, Ts...>::value;

}  // namespace yk::core

#endif  // YK_UTIL_EXACTLY_ONCE_HPP
