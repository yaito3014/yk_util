#ifndef YK_UTIL_FUNCTIONAL_HPP
#define YK_UTIL_FUNCTIONAL_HPP

#include "yk/util/type_list.hpp"

#include <type_traits>

namespace yk {

enum class invocable_kind {
  function,
  member_function,
  data_member,
  function_object,
  unknown,
};

namespace detail {

template <class F, class = void>
struct invocable_traits {
  static constexpr invocable_kind kind = invocable_kind::unknown;
};

template <class R, class... Args>
struct invocable_traits<R(Args...)> {
  static constexpr invocable_kind kind = invocable_kind::function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class R, class... Args>
struct invocable_traits<R (*)(Args...)> {
  static constexpr invocable_kind kind = invocable_kind::function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class R, class... Args>
struct invocable_traits<R (&)(Args...)> {
  static constexpr invocable_kind kind = invocable_kind::function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class C, class R>
struct invocable_traits<R C::*> {
  static constexpr invocable_kind kind = invocable_kind::data_member;
  using return_type = R;
  using parameters = type_list<>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...)> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) noexcept> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) const> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) const noexcept> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) volatile> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) volatile noexcept> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) const volatile> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) const volatile noexcept> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<Args...>;
};

template <class F>
struct invocable_traits<F, std::void_t<decltype(&F::operator())>> {
  static constexpr invocable_kind kind = invocable_kind::function_object;
  using return_type = typename invocable_traits<decltype(&F::operator())>::return_type;
  using parameters = typename invocable_traits<decltype(&F::operator())>::parameters;
};

template <std::size_t N, class F, class TypeList>
struct is_n_ary_function_impl;

template <std::size_t N, class F, class... Args>
struct is_n_ary_function_impl<N, F, type_list<Args...>> : std::bool_constant<N == sizeof...(Args)> {};

template <std::size_t N, class F, class = void>
struct is_n_ary_function;

template <std::size_t N, class F>
struct is_n_ary_function<N, F, std::enable_if_t<invocable_traits<F>::kind == invocable_kind::unknown>>
    : std::true_type {};

template <std::size_t N, class F>
struct is_n_ary_function<N, F, std::enable_if_t<invocable_traits<F>::kind != invocable_kind::unknown>>
    : is_n_ary_function_impl<N, F, typename invocable_traits<F>::parameters> {};

}  // namespace detail

template <class F>
struct invocable_traits : detail::invocable_traits<F> {};

template <class F>
struct invocable_traits<const F> : invocable_traits<F> {};

template <std::size_t N, class F>
struct is_n_ary_function : detail::is_n_ary_function<N, F> {};

template <class F>
using is_unary_function = is_n_ary_function<1, F>;

template <class F>
using is_binary_function = is_n_ary_function<2, F>;

template <std::size_t N, class F>
inline constexpr bool is_n_ary_function_v = is_n_ary_function<N, F>::value;

template <class F>
inline constexpr bool is_unary_function_v = is_unary_function<F>::value;

template <class F>
inline constexpr bool is_binary_function_v = is_binary_function<F>::value;

}  // namespace yk

#endif  // YK_UTIL_FUNCTIONAL_HPP
