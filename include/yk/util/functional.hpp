#ifndef YK_UTIL_FUNCTIONAL_HPP
#define YK_UTIL_FUNCTIONAL_HPP

#include "yk/util/type_list.hpp"
#include "yk/no_unique_address.hpp"

#include <functional>
#include <type_traits>
#include <utility>

namespace yk {

enum class invocable_kind {
  function,
  member_function,
  data_member,
  function_object,
  generic_function_object,
};

namespace detail {

template <class F, class = void>
struct invocable_traits {
  static constexpr invocable_kind kind = invocable_kind::generic_function_object;
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
  using parameters = type_list<C&>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...)> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<C&, Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) noexcept> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<C&, Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) const> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<const C&, Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) const noexcept> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<const C&, Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) volatile> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<volatile C&, Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) volatile noexcept> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<volatile C&, Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) const volatile> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<const volatile C&, Args...>;
};

template <class C, class R, class... Args>
struct invocable_traits<R (C::*)(Args...) const volatile noexcept> {
  static constexpr invocable_kind kind = invocable_kind::member_function;
  using return_type = R;
  using parameters = type_list<const volatile C&, Args...>;
};

template <class TypeList>
struct drop_head {};

template <class T, class... Args>
struct drop_head<type_list<T, Args...>> {
  using type = type_list<Args...>;
};

template <class F>
struct invocable_traits<F, std::void_t<decltype(&F::operator())>> {
  static constexpr invocable_kind kind = invocable_kind::function_object;
  using return_type = typename invocable_traits<decltype(&F::operator())>::return_type;
  using parameters = typename drop_head<typename invocable_traits<decltype(&F::operator())>::parameters>::type;
};

template <class F, std::size_t N, class TypeList>
struct is_n_ary_function_impl;

template <class F, std::size_t N, class... Args>
struct is_n_ary_function_impl<F, N, type_list<Args...>> : std::bool_constant<N == sizeof...(Args)> {};

template <class F, std::size_t N, class = void>
struct is_n_ary_function;

template <class F, std::size_t N>
struct is_n_ary_function<F, N, std::enable_if_t<invocable_traits<F>::kind == invocable_kind::generic_function_object>>
    : std::true_type {};

template <class F, std::size_t N>
struct is_n_ary_function<F, N, std::enable_if_t<invocable_traits<F>::kind != invocable_kind::generic_function_object>>
    : is_n_ary_function_impl<F, N, typename invocable_traits<F>::parameters> {};

}  // namespace detail

template <class F>
struct invocable_traits : detail::invocable_traits<F> {};

template <class F>
struct invocable_traits<const F> : invocable_traits<F> {};

template <class F, std::size_t N>
struct is_n_ary_function : detail::is_n_ary_function<F, N> {};

template <class F>
using is_unary_function = is_n_ary_function<F, 1>;

template <class F>
using is_binary_function = is_n_ary_function<F, 2>;

template <class F, std::size_t N>
inline constexpr bool is_n_ary_function_v = is_n_ary_function<F, N>::value;

template <class F>
inline constexpr bool is_unary_function_v = is_unary_function<F>::value;

template <class F>
inline constexpr bool is_binary_function_v = is_binary_function<F>::value;

template <class F, std::size_t N>
concept n_ary_function = is_n_ary_function_v<F, N>;

template <class F>
concept unary_function = n_ary_function<F, 1>;

template <class F>
concept binary_function = n_ary_function<F, 2>;

namespace detail {

template <class F, class... Fs>
struct compose_impl;
template <class F>
struct compose_impl<F> {
  YK_NO_UNIQUE_ADDRESS F func;

  constexpr compose_impl(F&& f) noexcept : func(std::forward<F>(f)) {}

  template <class... Args>
  constexpr decltype(auto) operator()(Args&&... args) const noexcept(noexcept(std::is_nothrow_invocable_v<F, Args...>))
  {
    return std::invoke(func, std::forward<Args>(args)...);
  }
};

template <class F, class G, class... Fs>
struct compose_impl<F, G, Fs...> {
  YK_NO_UNIQUE_ADDRESS F func;
  YK_NO_UNIQUE_ADDRESS compose_impl<G, Fs...> composed;

  constexpr compose_impl(F&& f, G&& g, Fs&&... fs) noexcept
      : func(std::forward<F>(f)), composed(std::forward<G>(g), std::forward<Fs>(fs)...)
  {
  }

  template <class... Args>
  constexpr decltype(auto) operator()(Args&&... args) const noexcept(
      noexcept(std::invoke(func, std::invoke(composed, std::forward<Args>(args)...)))
  )
  {
    return std::invoke(func, std::invoke(composed, std::forward<Args>(args)...));
  }
};

}  // namespace detail

template <class F, class... Fs>
constexpr auto compose(F&& f, Fs&&... fs) noexcept
{
  return detail::compose_impl<F, Fs...>{std::forward<F>(f), std::forward<Fs>(fs)...};
}

}  // namespace yk

#endif  // YK_UTIL_FUNCTIONAL_HPP
