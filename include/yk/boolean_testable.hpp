#ifndef YK_BOOLEAN_TESTABLE_HPP
#define YK_BOOLEAN_TESTABLE_HPP

#include <concepts>

namespace yk {

namespace detail {

template <class T>
concept boolean_testable_impl = std::convertible_to<T, bool>;

}  // namespace detail

template <class T>
concept boolean_testable = detail::boolean_testable_impl<T> && requires(T&& t) {
  { !static_cast<T&&>(t) } -> detail::boolean_testable_impl;
};

}  // namespace yk

#endif  // YK_BOOLEAN_TESTABLE_HPP
