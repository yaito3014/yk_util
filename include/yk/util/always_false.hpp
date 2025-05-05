#ifndef YK_UTIL_ALWAYS_FALSE_HPP
#define YK_UTIL_ALWAYS_FALSE_HPP

#include <type_traits>

namespace yk {

namespace core {

template <class... Ts>
struct always_false : std::false_type {};

template <class... Ts>
inline constexpr bool always_false_v = always_false<Ts...>::value;

}  // namespace core

}  // namespace yk

#endif  // YK_UTIL_ALWAYS_FALSE_HPP
