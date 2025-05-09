#ifndef YK_COMPARE_COMPARATOR
#define YK_COMPARE_COMPARATOR

#include "yk/compare/concepts.hpp"
#include "yk/util/functional.hpp"
#include "yk/util/specialization_of.hpp"
#include "yk/no_unique_address.hpp"

#include <compare>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace yk {

namespace compare {

struct comparator_interface {};

template <class Comp>
inline constexpr bool enable_comparator = std::derived_from<Comp, comparator_interface>;

template <class Comp>
concept comparator = yk::binary_function<Comp> && enable_comparator<Comp>;

struct comparator_adaptor_closure {};

template <class Comp, std::derived_from<comparator_adaptor_closure> Closure>
constexpr auto operator|(Comp comp, Closure closure) noexcept
{
  return std::invoke(std::forward<Closure>(closure), std::move(comp));
}

template <class Comp>
struct wrapper_comparator : comparator_interface {
  YK_NO_UNIQUE_ADDRESS Comp comp;

  constexpr wrapper_comparator(Comp c) noexcept : comp(std::move(c)) {}

  template <class T, class U>
  constexpr auto operator()(T&& x, U&& y) const noexcept(noexcept(std::is_nothrow_invocable_v<Comp, T, U>))
  {
    return std::invoke(comp, std::forward<T>(x), std::forward<U>(y));
  }
};

namespace detail {

struct wrap_fn {
  template <binary_function Comp>
  constexpr auto operator()(Comp comp) const noexcept
  {
    return wrapper_comparator{std::move(comp)};
  }

  template <comparator Comp>
  constexpr auto operator()(Comp comp) const noexcept
  {
    return comp;
  }
};

}  // namespace detail

namespace comparators {

inline constexpr detail::wrap_fn wrap{};

}  // namespace comparators

template <class Comp>
using wrap_t = decltype(comparators::wrap(std::declval<Comp>()));

template <comparator Comp1, comparator Comp2>
struct then_comparator : comparator_interface {
  YK_NO_UNIQUE_ADDRESS Comp1 comp1;
  YK_NO_UNIQUE_ADDRESS Comp2 comp2;

  constexpr then_comparator(Comp1 c1, Comp2 c2) noexcept : comp1(std::move(c1)), comp2(std::move(c2)) {}

  template <class T, class U>
  constexpr auto operator()(T&& x, U&& y) const noexcept(
      std::is_nothrow_invocable_v<Comp1, T, U> && std::is_nothrow_invocable_v<Comp2, T, U>
  ) -> std::invoke_result_t<Comp2, T, U>
  {
    ordering auto res = std::invoke(comp1, std::forward<T>(x), std::forward<U>(y));
    if (res == 0) {
      return std::invoke(comp2, std::forward<T>(x), std::forward<U>(y));
    } else {
      return res;
    }
  }
};

template <class Comp1, class Comp2>
then_comparator(Comp1, Comp2) -> then_comparator<wrap_t<Comp1>, wrap_t<Comp2>>;

namespace detail {

template <class Comp2>
struct comp_then_closure : comparator_adaptor_closure {
  YK_NO_UNIQUE_ADDRESS Comp2 comp2;

  constexpr comp_then_closure(Comp2 c2) noexcept : comp2(c2) {}

  template <class Comp1>
  constexpr auto operator()(Comp1 comp1) const noexcept
  {
    return then_comparator{std::move(comp1), std::move(comp2)};
  }
};

struct comp_then_fn {
  template <class Comp1, class Comp2>
  constexpr auto operator()(Comp1 comp1, Comp2 comp2) const noexcept
  {
    return then_comparator{std::move(comp1), std::move(comp2)};
  }

  template <class Comp2>
  constexpr auto operator()(Comp2 comp2) const noexcept
  {
    return comp_then_closure{std::move(comp2)};
  }
};

}  // namespace detail

namespace comparators {

inline constexpr detail::comp_then_fn then{};

}  // namespace comparators

template <unary_function F>
struct extract_comparator : comparator_interface {
  YK_NO_UNIQUE_ADDRESS F func;

  constexpr extract_comparator(F f) noexcept : func(std::move(f)) {}

  template <class T, class U>
  constexpr auto operator()(T&& x, U&& y) const noexcept(
      noexcept(std::compare_three_way{}(std::invoke(func, std::forward<T>(x)), std::invoke(func, std::forward<U>(y))))
  )
  {
    return std::compare_three_way{}(std::invoke(func, std::forward<T>(x)), std::invoke(func, std::forward<U>(y)));
  }
};

namespace detail {

struct extract_fn {
  template <class F>
  constexpr auto operator()(F func) const noexcept
  {
    return extract_comparator{func};
  }
};

}  // namespace detail

namespace comparators {

inline constexpr detail::extract_fn extract{};

}  // namespace comparators

// short-hand syntax
template <comparator Comp, unary_function F>
  requires(!std::derived_from<F, comparator_adaptor_closure>)
constexpr auto operator|(Comp comp, F f) noexcept
{
  return comp | comparators::then(comparators::extract(f));
}

}  // namespace compare

namespace comparators = compare::comparators;

}  // namespace yk

#endif  // YK_COMPARE_COMPARATOR
