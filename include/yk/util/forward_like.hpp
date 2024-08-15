#ifndef YK_UTIL_FORWARD_LIKE_HPP
#define YK_UTIL_FORWARD_LIKE_HPP

#include <type_traits>

namespace yk {

template <class From, class To>
struct copy_const : std::conditional<std::is_const_v<std::remove_reference_t<From>>, const To, To> {};

template <class From, class To>
using copy_const_t = typename copy_const<From, To>::type;

template <class From, class To>
struct override_ref : std::enable_if_t<std::is_reference_v<From>, std::conditional<std::is_rvalue_reference_v<From>, std::remove_reference_t<To>&&, To&>> {};

template <class From, class To>
using override_ref_t = typename override_ref<From, To>::type;

template <class From, class To>
using forward_like_t = override_ref_t<From&&, copy_const_t<From, std::remove_reference_t<To>>>;

template <class From, class To>
[[nodiscard]] constexpr auto forward_like(To&& x) noexcept -> forward_like_t<From, To> {
  return static_cast<forward_like_t<From, To>>(x);
}

}  // namespace yk

#endif  // YK_UTIL_FORWARD_LIKE_HPP
