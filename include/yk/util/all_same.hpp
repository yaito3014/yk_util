#ifndef YK_UTIL_ALL_SAME_HPP
#define YK_UTIL_ALL_SAME_HPP

#include <type_traits>

namespace yk::core {

template <class... Ts>
struct is_all_same : std::true_type {};

template <class T, class... Us>
struct is_all_same<T, Us...> : std::conjunction<std::is_same<T, Us>...> {};

template <class... Ts>
inline constexpr bool is_all_same_v = is_all_same<Ts...>::value;

}  // namespace yk::core

#endif  // YK_UTIL_ALL_SAME_HPP
