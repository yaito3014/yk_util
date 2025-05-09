#ifndef YK_COMPARE_COMMON_ORDERING_HPP
#define YK_COMPARE_COMMON_ORDERING_HPP

#include "yk/compare/concepts.hpp"

#include <compare>
#include <type_traits>

namespace yk {

namespace compare {

template <ordering T, ordering U>
struct common_ordering : std::common_type<T, U> {};

template <ordering T, ordering U>
using common_ordering_t = typename common_ordering<T, U>::type;

}  // namespace compare

}  // namespace yk

#endif  // YK_COMPARE_COMMON_ORDERING_HPP
