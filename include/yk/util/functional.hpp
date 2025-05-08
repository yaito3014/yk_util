#ifndef YK_UTIL_FUNCTIONAL_HPP
#define YK_UTIL_FUNCTIONAL_HPP

#include "yk/util/type_list.hpp"

#include <type_traits>

namespace yk {

enum class function_kind {
  function,
  member_function,
  function_object,
  unknown,
};

namespace detail {

template <class F, class = void>
struct function_traits {
  static constexpr function_kind kind = function_kind::unknown;
};

template <class R, class... Args>
struct function_traits<R(Args...)> {
  static constexpr function_kind kind = function_kind::function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class R, class... Args>
struct function_traits<R (*)(Args...)> {
  static constexpr function_kind kind = function_kind::function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class R, class... Args>
struct function_traits<R (&)(Args...)> {
  static constexpr function_kind kind = function_kind::function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...)> {
  static constexpr function_kind kind = function_kind::member_function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class C, class R, class... Args>
struct function_traits<R (C::*)(Args...) const> {
  static constexpr function_kind kind = function_kind::member_function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class F>
struct function_traits<F, std::void_t<decltype(&F::operator())>> {
  static constexpr function_kind kind = function_kind::function_object;
  using return_type = typename function_traits<decltype(&F::operator())>::return_type;
  using parameters = typename function_traits<decltype(&F::operator())>::parameters;
};

template <std::size_t N, class F, class TypeList = typename function_traits<F>::parameters, class = void>
struct is_nary_function_impl;

template <std::size_t N, class F, class... Args>
struct is_nary_function_impl<
    N, F, type_list<Args...>, std::enable_if_t<function_traits<F>::kind != function_kind::unknown>>
    : std::bool_constant<N == sizeof...(Args)> {};

// fallback to true if function type is unknown
template <std::size_t N, class F, class TypeList>
struct is_nary_function_impl<N, F, TypeList, std::enable_if_t<function_traits<F>::kind == function_kind::unknown>>
    : std::true_type {};

}  // namespace detail

template <class F>
struct function_traits : detail::function_traits<F> {};

template <class F>
struct function_traits<const F> : function_traits<F> {};

template <std::size_t N, class F>
struct is_nary_function : detail::is_nary_function_impl<N, F> {};

template <class F>
using is_unary_function = is_nary_function<1, F>;

template <class F>
using is_binary_function = is_nary_function<2, F>;

template <std::size_t N, class F>
inline constexpr bool is_nary_function_v = is_nary_function<N, F>::value;

template <class F>
inline constexpr bool is_unary_function_v = is_unary_function<F>::value;

template <class F>
inline constexpr bool is_binary_function_v = is_binary_function<F>::value;

}  // namespace yk

#endif  // YK_UTIL_FUNCTIONAL_HPP
