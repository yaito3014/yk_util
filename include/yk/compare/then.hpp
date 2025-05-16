#ifndef YK_THEN_COMPARE_HPP
#define YK_THEN_COMPARE_HPP

#include "yk/compare/concepts.hpp"
#include "yk/no_unique_address.hpp"

#include <compare>
#include <functional>

namespace yk {

namespace compare {

namespace detail {

template <ordering T>
struct then_closure {
  T value;

  template <ordering Lhs>
  friend constexpr std::common_comparison_category_t<Lhs, T> operator|(Lhs lhs, then_closure rhs) noexcept
  {
    if (lhs == 0) {
      return rhs.value;
    } else {
      return lhs;
    }
  }
};

struct then_impl {
  template <ordering T>
  constexpr then_closure<T> operator()(T x) const noexcept
  {
    return {x};
  }

  template <ordering T, ordering U>
  constexpr auto operator()(T x, U y) const noexcept
  {
    return x | then_closure<U>{y};
  }
};

template <class F>
  requires std::invocable<std::decay_t<F>>
struct then_with_closure {
  YK_NO_UNIQUE_ADDRESS F func;

  template <ordering Lhs, class Self>
    requires std::same_as<std::remove_cvref_t<Self>, then_with_closure>
  friend constexpr auto operator|(Lhs lhs, Self&& rhs) noexcept
      -> std::common_comparison_category_t<Lhs, std::invoke_result_t<std::decay_t<F>>>
  {
    if (lhs == 0) {
      return std::invoke(std::forward<Self>(rhs).func);
    } else {
      return lhs;
    }
  }
};

struct then_with_impl {
  template <class F>
    requires std::invocable<std::decay_t<F>>
  constexpr then_with_closure<F> operator()(F&& f) const noexcept
  {
    return then_with_closure<F>{f};
  }

  template <ordering T, class F>
    requires std::invocable<std::decay_t<F>>
  constexpr auto operator()(T x, F&& f) const noexcept
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
