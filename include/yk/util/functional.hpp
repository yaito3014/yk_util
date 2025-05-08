#ifndef YK_UTIL_FUNCTIONAL_HPP
#define YK_UTIL_FUNCTIONAL_HPP

#include "yk/util/type_list.hpp"

#include <type_traits>

namespace yk {

enum class function_kind {
  function,
  member_function,
  function_object,
  unkown,
};

namespace detail {

template <class F, class = void>
struct function_traits {
  static constexpr function_kind kind = function_kind::unkown;
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

}  // namespace detail

template <class F>
struct function_traits : detail::function_traits<F> {};

template <class F>
struct function_traits<const F> : function_traits<F> {};

}  // namespace yk

#endif  // YK_UTIL_FUNCTIONAL_HPP