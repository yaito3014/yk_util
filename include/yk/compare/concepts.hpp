#ifndef YK_COMPARE_CONCEPTS_HPP
#define YK_COMPARE_CONCEPTS_HPP

#include <concepts>

namespace yk {

namespace compare {

template <class T>
concept ordering = requires(T x) {
  { x < 0 } -> std::same_as<bool>;
  { x == 0 } -> std::same_as<bool>;
  { x > 0 } -> std::same_as<bool>;
};

}  // namespace compare

}  // namespace yk

#endif  // YK_COMPARE_CONCEPTS_HPP
