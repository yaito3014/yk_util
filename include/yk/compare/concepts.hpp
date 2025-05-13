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
  { x <= 0 } -> boolean_testable;
  { x == 0 } -> boolean_testable;
  { x >= 0 } -> boolean_testable;
  { x > 0 } -> boolean_testable;
  { x != 0 } -> boolean_testable;
  { x <=> 0 } -> std::same_as<T>;
  { 0 < x } -> boolean_testable;
  { 0 <= x } -> boolean_testable;
  { 0 == x } -> boolean_testable;
  { 0 >= x } -> boolean_testable;
  { 0 > x } -> boolean_testable;
  { 0 != x } -> boolean_testable;
  { 0 <=> x } -> std::same_as<T>;
};

}  // namespace compare

}  // namespace yk

#endif  // YK_COMPARE_CONCEPTS_HPP
