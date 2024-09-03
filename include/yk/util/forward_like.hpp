#ifndef YK_UTIL_FORWARD_LIKE_HPP
#define YK_UTIL_FORWARD_LIKE_HPP

#include <type_traits>

namespace yk {

namespace detail {

template <class From, class To>
struct copy_const_impl {
  using type = To;
};

template <class From, class To>
struct copy_const_impl<From&, To> {
  using type = To;
};

template <class From, class To>
struct copy_const_impl<From&&, To> {
  using type = To;
};

template <class From, class To>
struct copy_const_impl<const From, To> {
  using type = const To;
};

template <class From, class To>
struct copy_const_impl<const From, To&> {
  using type = const To&;
};

template <class From, class To>
struct copy_const_impl<const From, To&&> {
  using type = const To&&;
};

template <class From, class To>
struct copy_const_impl<const From&, To> {
  using type = const To;
};

template <class From, class To>
struct copy_const_impl<const From&, To&> {
  using type = const To&;
};

template <class From, class To>
struct copy_const_impl<const From&, To&&> {
  using type = const To&&;
};

template <class From, class To>
struct copy_const_impl<const From&&, To> {
  using type = const To;
};

template <class From, class To>
struct copy_const_impl<const From&&, To&> {
  using type = const To&;
};

template <class From, class To>
struct copy_const_impl<const From&&, To&&> {
  using type = const To&&;
};

template <class From, class To>
struct override_ref_impl {
  using type = To&;
};

template <class From, class To>
struct override_ref_impl<From&&, To> {
  using type = To&&;
};

template <class From, class To>
struct override_ref_impl<From&&, To&> {
  using type = To&&;
};

template <class From, class To>
struct override_ref_impl<From&&, To&&> {
  using type = To&&;
};

template <class From, class To>
struct forward_like_impl;  // From must be reference, To must be lvalue reference

template <class From, class To>
struct forward_like_impl<From&, To&> {
  using type = To&;
};

template <class From, class To>
struct forward_like_impl<From&&, To&> {
  using type = To&&;
};

template <class From, class To>
struct forward_like_impl<const From&, To&> {
  using type = const To&;
};

template <class From, class To>
struct forward_like_impl<const From&&, To&> {
  using type = const To&&;
};

}  // namespace detail

template <class From, class To>
struct copy_const : detail::copy_const_impl<From, To> {};

template <class From, class To>
using copy_const_t = typename copy_const<From, To>::type;

template <class From, class To>
struct override_ref : detail::override_ref_impl<From, To> {};

template <class From, class To>
using override_ref_t = typename override_ref<From, To>::type;

template <class From, class To>
using forward_like_t = typename detail::forward_like_impl<From&&, To&>::type;

template <class From, class To>
[[nodiscard]] constexpr forward_like_t<From, To> forward_like(To&& x) noexcept {
  return static_cast<forward_like_t<From, To>>(x);
}

}  // namespace yk

#endif  // YK_UTIL_FORWARD_LIKE_HPP
