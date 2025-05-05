#ifndef YK_UTIL_FIND_TYPE_INDEX_HPP
#define YK_UTIL_FIND_TYPE_INDEX_HPP

#include <type_traits>

namespace yk::core {

namespace detail {

template <std::size_t I, std::size_t N, class T, class... Us>
struct find_type_index_impl : std::integral_constant<std::size_t, N> {};

template <std::size_t I, std::size_t N, class T, class U, class... Us>
struct find_type_index_impl<I, N, T, U, Us...>
    : std::conditional_t<std::is_same_v<T, U>, std::integral_constant<std::size_t, I>, find_type_index_impl<I + 1, N, T, Us...> > {};

}  // namespace detail

template <class T, class... Ts>
struct find_type_index : detail::find_type_index_impl<0, sizeof...(Ts), T, Ts...> {};

template <class T, class... Ts>
inline constexpr std::size_t find_type_index_v = find_type_index<T, Ts...>::value;

}  // namespace yk::core

#endif  // YK_UTIL_FIND_TYPE_INDEX_HPP
