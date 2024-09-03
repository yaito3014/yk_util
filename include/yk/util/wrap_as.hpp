#ifndef YK_UTIL_WRAP_AS_HPP
#define YK_UTIL_WRAP_AS_HPP

#include <utility>

namespace yk {

namespace detail {

template <class T, class U>
struct wrap_as_impl {
  using type = T;
};

template <class T, class U>
struct wrap_as_impl<const T, U>;  // ill-formed

template <class T, class U>
struct wrap_as_impl<T&, U>;  // ill-formed

template <class T, class U>
struct wrap_as_impl<T&&, U>;  // ill-formed

template <class T>
struct wrap_as_impl<T, T> {
  using type = T&&;
};

template <class T>
struct wrap_as_impl<T, T&> {
  using type = T&;
};

template <class T>
struct wrap_as_impl<T, T&&> {
  using type = T&&;
};

template <class T>
struct wrap_as_impl<T, const T> {
  using type = const T&&;
};

template <class T>
struct wrap_as_impl<T, const T&> {
  using type = const T&;
};

template <class T>
struct wrap_as_impl<T, const T&&> {
  using type = const T&&;
};

}  // namespace detail

template <class T, class U>
using wrap_as_t = typename detail::wrap_as_impl<T, U>::type;

template <class T, class U>
[[nodiscard]] constexpr wrap_as_t<T, U> wrap_as(U&& x) noexcept(noexcept(static_cast<wrap_as_t<T, U>>(std::forward<U>(x)))) {
  return static_cast<wrap_as_t<T, U>>(std::forward<U>(x));
}

}  // namespace yk

#endif  // YK_UTIL_WRAP_AS_HPP
