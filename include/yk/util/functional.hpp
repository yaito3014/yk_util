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
  overloaded_function_object,
};

namespace detail {

template <class T, class = void>
struct has_unambiguous_function_call_operator : std::false_type {};

template <class T>
struct has_unambiguous_function_call_operator<T, std::void_t<decltype(&T::operator())>> : std::true_type {};

struct has_function_call_operator_mixin {
  void operator()() {}
};

template <class T>
struct has_function_call_operator_helper : T, has_function_call_operator_mixin {};

template <class T>
struct has_function_call_operator
    : std::conjunction<
          std::is_class<T>,
          std::negation<has_unambiguous_function_call_operator<has_function_call_operator_helper<T>>>> {};

}  // namespace detail

template <class T>
struct has_unambiguous_function_call_operator : detail::has_unambiguous_function_call_operator<T> {};

template <class T>
inline constexpr bool has_umambiguous_function_call_operator_v = has_unambiguous_function_call_operator<T>::value;

template <class T>
struct has_function_call_operator : detail::has_function_call_operator<T> {};

template <class T>
inline constexpr bool has_function_call_operator_v = has_function_call_operator<T>::value;

namespace detail {

template <class F, class = void>
struct invocable_traits {};

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
struct invocable_traits<
    F, std::enable_if_t<has_function_call_operator_v<F> && has_umambiguous_function_call_operator_v<F>>> {
  static constexpr invocable_kind kind = invocable_kind::function_object;
  using return_type = typename invocable_traits<decltype(&F::operator())>::return_type;
  using parameters = typename drop_head<typename invocable_traits<decltype(&F::operator())>::parameters>::type;
};

template <class F>
struct invocable_traits<
    F, std::enable_if_t<has_function_call_operator_v<F> && !has_umambiguous_function_call_operator_v<F>>> {
  static constexpr invocable_kind kind = invocable_kind::overloaded_function_object;
};

template <class T, class = void>
struct has_arity : std::false_type {};

template <class T>
struct has_arity<T, std::void_t<decltype(T::arity)>> : std::true_type {};

template <class F, std::size_t N, class TypeList, class = void>
struct is_n_ary_function_impl : std::bool_constant<N == TypeList::size> {};

template <class F, std::size_t N, class TypeList>
struct is_n_ary_function_impl<F, N, TypeList, std::enable_if_t<has_arity<F>::value>>
    : std::bool_constant<N == TypeList::size> {
  static_assert(F::arity == TypeList::size, "arity must match parameters' size");
};

template <class F, std::size_t N, class = void>
struct is_n_ary_function : std::false_type {};

template <class F, std::size_t N>
struct is_n_ary_function<
    F, N,
    std::enable_if_t<invocable_traits<F>::kind == invocable_kind::overloaded_function_object && !has_arity<F>::value>>
    : std::true_type {};

template <class F, std::size_t N>
struct is_n_ary_function<
    F, N,
    std::enable_if_t<invocable_traits<F>::kind == invocable_kind::overloaded_function_object && has_arity<F>::value>>
    : std::bool_constant<N == F::arity> {};

template <class F, std::size_t N>
struct is_n_ary_function<
    F, N, std::enable_if_t<invocable_traits<F>::kind != invocable_kind::overloaded_function_object>>
    : is_n_ary_function_impl<F, N, typename invocable_traits<F>::parameters> {};

}  // namespace detail

template <class F>
struct invocable_traits : detail::invocable_traits<F> {};

template <class F>
struct invocable_traits<const F> : invocable_traits<F> {};

template <class F, std::size_t N>
struct is_n_ary_function : detail::is_n_ary_function<F, N> {};

template <class F, std::size_t N>
struct is_n_ary_function<const F, N> : is_n_ary_function<F, N> {};

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

  constexpr compose_impl(F f, G g, Fs... fs) noexcept : func(std::move(f)), composed(std::move(g), std::move(fs)...) {}

  template <class... Args>
  constexpr decltype(auto) operator()(Args&&... args) const noexcept(
      noexcept(std::invoke(func, std::invoke(composed, std::forward<Args>(args)...)))
  )
  {
    return std::invoke(func, std::invoke(composed, std::forward<Args>(args)...));
  }
};

template <class F, class... Fs>
compose_impl(F, Fs...) -> compose_impl<F, Fs...>;

}  // namespace detail

template <class F, class... Fs>
[[nodiscard]] constexpr auto compose(F&& f, Fs&&... fs) noexcept
{
  return detail::compose_impl{std::forward<F>(f), std::forward<Fs>(fs)...};
}

}  // namespace yk

#endif  // YK_UTIL_FUNCTIONAL_HPP
