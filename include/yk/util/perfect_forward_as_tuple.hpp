#ifndef YK_PERFECT_FORWARD_AS_TUPLE_HPP
#define YK_PERFECT_FORWARD_AS_TUPLE_HPP

#include <tuple>
#include <utility>

namespace yk {

// converts T&& to T, T& to T&
template <class... Args>
constexpr std::tuple<Args...> perfect_forward_as_tuple(Args&&... args)
    noexcept(noexcept(std::tuple<Args...>{std::forward<Args>(args)...}))
{
  return std::tuple<Args...>{std::forward<Args>(args)...};
}

}  // namespace yk

#endif  // YK_PERFECT_FORWARD_AS_TUPLE_HPP
