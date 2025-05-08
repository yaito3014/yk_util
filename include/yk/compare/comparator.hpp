#ifndef YK_COMPARE_COMPARATOR
#define YK_COMPARE_COMPARATOR

#include "yk/compare/concepts.hpp"
#include "yk/util/specialization_of.hpp"
#include "yk/no_unique_address.hpp"

#include <compare>
#include <functional>
#include <utility>

namespace yk {

namespace comparators {

namespace detail {

template <class Comp>
struct then_closure {
  YK_NO_UNIQUE_ADDRESS Comp comp;

  template <class AnotherComp>
  friend constexpr auto operator|(AnotherComp another, then_closure closure) noexcept
  {
    return [=]<class T, class U>(T&& x, U&& y) {
      const compare::ordering auto res = std::invoke(another, std::forward<T>(x), std::forward<U>(y));
      if (res == 0) {
        return std::invoke(closure.comp, std::forward<T>(x), std::forward<U>(y));
      } else {
        return res;
      }
    };
  }
};

struct then_impl {
  template <class Comp>
  constexpr then_closure<Comp> operator()(Comp comp) const noexcept
  {
    return {comp};
  }

  template <class Comp1, class Comp2>
  constexpr auto operator()(Comp1 comp1, Comp2 comp2) const noexcept
  {
    return comp1 | then_closure<Comp2>{comp2};
  }
};

}  // namespace detail

inline constexpr detail::then_impl then{};

template <class F>
struct extract {
  YK_NO_UNIQUE_ADDRESS F func;

  template <class T, class U>
  constexpr auto operator()(T&& x, U&& y) const noexcept(
      noexcept(std::compare_three_way{}(std::invoke(func, std::forward<T>(x)), std::invoke(func, std::forward<U>(y))))
  )
  {
    return std::compare_three_way{}(std::invoke(func, std::forward<T>(x)), std::invoke(func, std::forward<U>(y)));
  }

  // short-hand syntax
  template <class G>
    requires(!specialization_of<G, detail::then_closure>)
  friend constexpr auto operator|(extract e, G g) noexcept
  {
    return e | then(extract<G>{g});
  }
};

}  // namespace comparators

}  // namespace yk

#endif  // YK_COMPARE_COMPARATOR
