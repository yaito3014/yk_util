#ifndef YK_UTIL_SPECIALIZATION_OF_HPP
#define YK_UTIL_SPECIALIZATION_OF_HPP

#include <type_traits>

namespace yk {

template <class T, template <class...> class TT>
struct is_specialization_of : std::false_type {};

template <template <class...> class TT, class... Ts>
struct is_specialization_of<TT<Ts...>, TT> : std::true_type {};

template <class T, template <class...> class TT>
inline constexpr bool is_specialization_of_v = is_specialization_of<T, TT>::value;

template <class T, template <class...> class TT>
concept specialization_of = is_specialization_of_v<T, TT>;

}  // namespace yk

#endif  // YK_UTIL_SPECIALIZATION_OF_HPP
