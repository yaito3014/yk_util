#ifndef YK_COMPARE_CONCEPTS_HPP
#define YK_COMPARE_CONCEPTS_HPP

#include "yk/boolean_testable.hpp"
#include "yk/util/functional.hpp"

#include <concepts>

namespace yk {

namespace compare {

template <class T>
concept ordering = requires(T x) {
  { x < 0 } -> boolean_testable;
  { x == 0 } -> boolean_testable;
  { x > 0 } -> boolean_testable;
};

template <class Proj>
concept projection = std::destructible<Proj> && yk::is_unary_function_v<Proj>;

template <class Proj, class T>
concept projection_for = std::invocable<Proj, T>;

template <class Comp>
concept comparator = std::destructible<Comp> && yk::is_binary_function_v<Comp>;

template <class Comp, class T, class U>
concept comparator_for = comparator<Comp> && std::invocable<Comp, T, U> && ordering<std::invoke_result_t<Comp, T, U>>;

}  // namespace compare

}  // namespace yk

#endif  // YK_COMPARE_CONCEPTS_HPP
