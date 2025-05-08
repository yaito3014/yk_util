#ifndef YK_THEN_COMPARE_HPP
#define YK_THEN_COMPARE_HPP

#include "yk/compare/concepts.hpp"

#include <compare>

namespace yk {

namespace compare {

namespace detail {

template <comparison T>
struct then_closure {
  T value;

  template <comparison Lhs>
    requires std::convertible_to<Lhs, T>
  friend constexpr T operator|(Lhs lhs, then_closure rhs) noexcept
  {
    if (lhs == 0) {
      return rhs.value;
    } else {
      return lhs;
    }
  }
};

struct then_impl {
  template <comparison T>
  constexpr then_closure<T> operator()(T x) const noexcept
  {
    return {x};
  }

  template <comparison T, comparison U>
  constexpr auto operator()(T x, U y) const noexcept
  {
    return x | then_closure<U>{y};
  }
};

template <std::invocable F>
struct then_with_closure {
  F func;

  template <comparison Lhs>
  friend constexpr std::invoke_result_t<F> operator|(Lhs lhs, then_with_closure rhs) noexcept
  {
    if (lhs == 0) {
      return rhs.func();
    } else {
      return lhs;
    }
  }
};

struct then_with_impl {
  template <std::invocable F>
  constexpr then_with_closure<F> operator()(F x) const noexcept
  {
    return {x};
  }

  template <comparison T, std::invocable F>
  constexpr auto operator()(T x, F f) const noexcept
  {
    return x | then_with_closure<F>{f};
  }
};

}  // namespace detail

inline constexpr detail::then_impl then{};
inline constexpr detail::then_with_impl then_with;

}  // namespace compare

}  // namespace yk

#endif  // YK_THEN_COMPARE_HPP
