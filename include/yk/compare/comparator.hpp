#ifndef YK_COMPARE_COMPARATOR
#define YK_COMPARE_COMPARATOR

#include "yk/compare/concepts.hpp"
#include "yk/no_unique_address.hpp"
#include "yk/util/functional.hpp"
#include "yk/util/specialization_of.hpp"

#include <compare>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace yk {

namespace compare {

namespace detail {

struct dummy_range {
  int* begin();
  int* end();
};

// is there more better way to detect range adaptor closure? idk
template <class Closure>
concept RangeAdaptorClosure = requires(Closure closure) {
  closure(dummy_range{});
  dummy_range{} | closure;
  closure | closure;
};

}  // namespace detail

struct comparator_interface {};

template <class Comp>
inline constexpr bool enable_comparator = std::derived_from<Comp, comparator_interface>;

template <class Comp>
concept comparator = yk::binary_function<Comp> && enable_comparator<Comp>;

struct comparator_adaptor_closure {};

template <class Comp, class Closure>
  requires binary_function<std::decay_t<Comp>>
           && std::derived_from<std::remove_cvref_t<Closure>, comparator_adaptor_closure>
[[nodiscard]] constexpr auto operator|(Comp&& comp, Closure&& closure) noexcept
{
  return std::invoke(std::forward<Closure>(closure), std::forward<Comp>(comp));
}

namespace detail {

template <class Lhs, class Rhs>
struct [[nodiscard]] pipe_closure : comparator_adaptor_closure {
  static constexpr std::size_t arity = 1;

  YK_NO_UNIQUE_ADDRESS Lhs lhs;
  YK_NO_UNIQUE_ADDRESS Rhs rhs;

  constexpr pipe_closure(Lhs&& l, Rhs&& r) noexcept : lhs(std::forward<Lhs>(l)), rhs(std::forward<Rhs>(r)) {}

  template <class Comp>
    requires binary_function<std::decay_t<Comp>>
  [[nodiscard]] constexpr auto operator()(Comp&& comp) const noexcept(
      noexcept(std::invoke(rhs, std::invoke(lhs, std::forward<Comp>(comp))))
  )
  {
    return std::invoke(rhs, std::invoke(lhs, std::forward<Comp>(comp)));
  }
};

}  // namespace detail

template <class Lhs, class Rhs>
  requires std::derived_from<std::remove_cvref_t<Lhs>, comparator_adaptor_closure>
           && std::derived_from<std::remove_cvref_t<Rhs>, comparator_adaptor_closure>
[[nodiscard]] constexpr auto operator|(Lhs&& lhs, Rhs&& rhs) noexcept
{
  return detail::pipe_closure{std::forward<Lhs>(lhs), std::forward<Rhs>(rhs)};
}

template <class Comp>
  requires binary_function<std::decay_t<Comp>>
struct wrapper_comparator : comparator_interface {
  static constexpr std::size_t arity = 2;

  YK_NO_UNIQUE_ADDRESS Comp comp;

  constexpr wrapper_comparator(Comp&& c) noexcept : comp(std::forward<Comp>(c)) {}

  template <class T, class U>
  [[nodiscard]] constexpr auto operator()(T&& x, U&& y) const noexcept(
      noexcept(std::is_nothrow_invocable_v<Comp, T, U>)
  )
  {
    return std::invoke(comp, std::forward<T>(x), std::forward<U>(y));
  }
};

template <class Comp>
wrapper_comparator(Comp&&) -> wrapper_comparator<Comp>;

namespace detail {

struct wrap_fn {
  template <class Comp>
    requires binary_function<std::decay_t<Comp>>
  [[nodiscard]] constexpr auto operator()(Comp&& comp) const noexcept
  {
    return wrapper_comparator{std::forward<Comp>(comp)};
  }

  template <class Comp>
    requires comparator<std::decay_t<Comp>>
  [[nodiscard]] constexpr auto operator()(Comp&& comp) const noexcept
  {
    return std::forward<Comp>(comp);
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
  static constexpr std::size_t arity = 2;

  YK_NO_UNIQUE_ADDRESS Comp1 comp1;
  YK_NO_UNIQUE_ADDRESS Comp2 comp2;

  template <class T, class U>
  constexpr then_comparator(T&& c1, U&& c2) noexcept : comp1(std::forward<T>(c1)), comp2(std::forward<U>(c2))
  {
  }

  template <class T, class U>
  [[nodiscard]] constexpr auto operator()(
      T&& x, U&& y
  ) const noexcept(std::is_nothrow_invocable_v<Comp1, T&, U&> && std::is_nothrow_invocable_v<Comp2, T&, U&>) ->
      typename std::common_comparison_category_t<
          std::invoke_result_t<Comp1, T&, U&>, std::invoke_result_t<Comp2, T&, U&>>
  {
    ordering auto res = std::invoke(comp1, x, y);
    if (res == 0) {
      return std::invoke(comp2, x, y);
    } else {
      return res;
    }
  }
};

template <class Comp1, class Comp2>
then_comparator(Comp1&&, Comp2&&) -> then_comparator<wrap_t<Comp1>, wrap_t<Comp2>>;

namespace detail {

template <class Comp2>
struct [[nodiscard]] comp_then_closure : comparator_adaptor_closure {
  static constexpr std::size_t arity = 1;

  YK_NO_UNIQUE_ADDRESS Comp2 comp2;

  constexpr comp_then_closure(Comp2&& c2) noexcept : comp2(std::forward<Comp2>(c2)) {}

  // if closure if const lvalue reference, pass reference
  template <class Comp1>
    requires binary_function<std::decay_t<Comp1>>
  [[nodiscard]] constexpr auto operator()(Comp1&& comp1) const& noexcept
  {
    return then_comparator{std::forward<Comp1>(comp1), comp2};
  }

  // if closure if non-const lvalue reference, pass reference
  template <class Comp1>
    requires binary_function<std::decay_t<Comp1>>
  [[nodiscard]] constexpr auto operator()(Comp1&& comp1) & noexcept
  {
    return then_comparator{std::forward<Comp1>(comp1), comp2};
  }

  // if closure if rvalue reference, move if Comp2 is not reference
  template <class Comp1>
    requires binary_function<std::decay_t<Comp1>>
  [[nodiscard]] constexpr auto operator()(Comp1&& comp1) && noexcept
  {
    return then_comparator{std::forward<Comp1>(comp1), std::forward<Comp2>(comp2)};
  }

  // template <class Self, class Comp1>
  // constexpr auto operator()(this Self&& self, Comp1&& comp1) noexcept
  // {
  //   return then_comparator{std::forward<Comp1>(comp1), move_if_both_rvalue<Self>(std::forward<Self>(self).comp2)};
  // }
};

template <class Comp2>
comp_then_closure(Comp2&&) -> comp_then_closure<Comp2>;

struct comp_then_fn {
  template <class Comp1, class Comp2>
    requires binary_function<std::decay_t<Comp1>> && binary_function<std::decay_t<Comp2>>
  [[nodiscard]] constexpr auto operator()(Comp1&& comp1, Comp2&& comp2) const noexcept
  {
    return then_comparator{std::forward<Comp1>(comp1), std::forward<Comp2>(comp2)};
  }

  template <class Comp2>
    requires binary_function<std::decay_t<Comp2>>
  [[nodiscard]] constexpr auto operator()(Comp2&& comp2) const noexcept
  {
    return comp_then_closure{std::forward<Comp2>(comp2)};
  }
};

}  // namespace detail

namespace comparators {

inline constexpr detail::comp_then_fn then{};

}  // namespace comparators

template <class From, class To>
struct is_promotable : std::false_type {};

template <>
struct is_promotable<std::partial_ordering, std::partial_ordering> : std::true_type {};

template <>
struct is_promotable<std::partial_ordering, std::weak_ordering> : std::true_type {};

template <>
struct is_promotable<std::partial_ordering, std::strong_ordering> : std::true_type {};

template <>
struct is_promotable<std::weak_ordering, std::weak_ordering> : std::true_type {};

template <>
struct is_promotable<std::weak_ordering, std::strong_ordering> : std::true_type {};

template <class From, class To>
inline constexpr bool is_promotable_v = is_promotable<From, To>::value;

template <comparator Comp1, comparator Comp2>
struct promote_comparator : comparator_interface {
  static constexpr std::size_t arity = 2;

  YK_NO_UNIQUE_ADDRESS Comp1 comp1;
  YK_NO_UNIQUE_ADDRESS Comp2 comp2;

  constexpr promote_comparator(Comp1&& comp1, Comp2&& comp2)
      : comp1(std::forward<Comp1>(comp1)), comp2(std::forward<Comp2>(comp2))
  {
  }

  // Invalid promotion. If you're trying to promote strong_ordering to strong_ordering, use then() instead.
  template <class T, class U>
  void operator()(T&&, U&&) const = delete;

  template <class T, class U>
  [[nodiscard]] constexpr auto operator()(T&& x, U&& y) const noexcept(
      std::is_nothrow_invocable_v<Comp1, T&, U&> && std::is_nothrow_invocable_v<Comp2, T&, U&>
  ) -> std::invoke_result_t<Comp2, T&, U&>
    requires is_promotable_v<std::invoke_result_t<Comp1, T&, U&>, std::invoke_result_t<Comp2, T&, U&>>
  {
    using From = std::invoke_result_t<Comp1, T&, U&>;
    using To = std::invoke_result_t<Comp2, T&, U&>;

    const auto res = std::invoke(comp1, x, y);

    if constexpr (std::same_as<From, To>) {
      if constexpr (std::same_as<From, std::partial_ordering>) {
        if (res == From::equivalent || res == From::unordered) return std::invoke(comp2, x, y);
        return res;

      } else {
        if (res == From::equivalent) return std::invoke(comp2, x, y);
        return res;
      }

    } else {
      if (res == From::less) return To::less;
      if (res == From::greater) return To::greater;
      return std::invoke(comp2, x, y);
    }
  }
};

template <class Comp1, class Comp2>
promote_comparator(Comp1&&, Comp2&&) -> promote_comparator<wrap_t<Comp1>, wrap_t<Comp2>>;

namespace detail {

template <class Comp2>
struct [[nodiscard]] promote_closure : comparator_adaptor_closure {
  static constexpr std::size_t arity = 1;

  YK_NO_UNIQUE_ADDRESS Comp2 comp2;

  [[nodiscard]] constexpr promote_closure(Comp2&& c2) noexcept : comp2(std::forward<Comp2>(c2)) {}

  template <class Comp1>
    requires binary_function<std::decay_t<Comp1>>
  [[nodiscard]] constexpr auto operator()(Comp1&& comp1) const& noexcept
  {
    return promote_comparator{std::forward<Comp1>(comp1), comp2};
  }

  template <class Comp1>
    requires binary_function<std::decay_t<Comp1>>
  [[nodiscard]] constexpr auto operator()(Comp1&& comp1) & noexcept
  {
    return promote_comparator{std::forward<Comp1>(comp1), comp2};
  }

  template <class Comp1>
    requires binary_function<std::decay_t<Comp1>>
  [[nodiscard]] constexpr auto operator()(Comp1&& comp1) && noexcept
  {
    return promote_comparator{std::forward<Comp1>(comp1), std::forward<Comp2>(comp2)};
  }
};

struct promote_fn {
  template <class Comp1, class Comp2>
    requires binary_function<std::decay_t<Comp1>> && binary_function<std::decay_t<Comp2>>
  [[nodiscard]] constexpr auto operator()(Comp1&& comp1, Comp2&& comp2) const noexcept
  {
    return promote_comparator{std::forward<Comp1>(comp1), std::forward<Comp2>(comp2)};
  }

  template <class Comp2>
    requires binary_function<std::decay_t<Comp2>>
  [[nodiscard]] constexpr auto operator()(Comp2&& comp2) const noexcept
  {
    return promote_closure<Comp2>{std::forward<Comp2>(comp2)};
  }
};

}  // namespace detail

namespace comparators {

inline constexpr detail::promote_fn promote{};

}  // namespace comparators

template <class F>
  requires unary_function<std::decay_t<F>>
struct extract_comparator : comparator_interface {
  static constexpr std::size_t arity = 2;

  YK_NO_UNIQUE_ADDRESS F func;

  constexpr extract_comparator(F&& f) noexcept : func(std::forward<F>(f)) {}

  template <class T, class U>
  [[nodiscard]] constexpr auto operator()(T&& x, U&& y) const noexcept(
      noexcept(std::compare_three_way{}(std::invoke(func, std::forward<T>(x)), std::invoke(func, std::forward<U>(y))))
  )
  {
    return std::compare_three_way{}(std::invoke(func, std::forward<T>(x)), std::invoke(func, std::forward<U>(y)));
  }
};

template <class Self, class RAC>
  requires specialization_of<std::remove_cvref_t<Self>, extract_comparator>
           && detail::RangeAdaptorClosure<std::remove_cvref_t<RAC>>
[[nodiscard]] constexpr auto operator|(Self&& comp, RAC&& rac) noexcept
{
  return extract_comparator{yk::compose(std::forward<RAC>(rac), std::forward<Self>(comp).func)};
}

namespace detail {

struct extract_fn {
  template <class F>
    requires unary_function<std::decay_t<F>>
  [[nodiscard]] constexpr auto operator()(F&& func) const noexcept
  {
    return extract_comparator{std::forward<F>(func)};
  }
};

}  // namespace detail

namespace comparators {

inline constexpr detail::extract_fn extract{};

}  // namespace comparators

// short-hand syntax
template <class Comp, class F>
  requires comparator<std::decay_t<Comp>> && unary_function<std::decay_t<F>>
           && (!std::derived_from<std::decay_t<F>, comparator_adaptor_closure>)
           && (!detail::RangeAdaptorClosure<std::decay_t<F>>)
[[nodiscard]] constexpr auto operator|(Comp&& comp, F&& f) noexcept
{
  return comparators::then(std::forward<Comp>(comp), comparators::extract(std::forward<F>(f)));
}

}  // namespace compare

namespace comparators = compare::comparators;

}  // namespace yk

#endif  // YK_COMPARE_COMPARATOR
