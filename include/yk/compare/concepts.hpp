#ifndef YK_COMPARE_CONCEPTS_HPP
#define YK_COMPARE_CONCEPTS_HPP

#include "yk/boolean_testable.hpp"

#include <concepts>

namespace yk {

namespace compare {

template <class T>
concept ordering = requires(T x) {
  { x < 0 } -> boolean_testable;
  { x == 0 } -> boolean_testable;
  { x > 0 } -> boolean_testable;
};

template <class Comp, class T, class U>
concept comparator = std::invocable<Comp, T, U> && ordering<std::invoke_result_t<Comp, T, U>>;

}  // namespace compare

}  // namespace yk

#endif  // YK_COMPARE_CONCEPTS_HPP
