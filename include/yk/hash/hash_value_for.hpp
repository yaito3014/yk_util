#ifndef YK_HASH_HASH_VALUE_FOR_HPP
#define YK_HASH_HASH_VALUE_FOR_HPP

#include "yk/hash/hash_value_for/std.hpp"

#if YK_UTIL_INCLUDE_BOOST
#include "yk/hash/hash_value_for/boost.hpp"
#include "yk/hash/range.hpp"
#endif

namespace yk {

template <class T>
[[nodiscard]] constexpr std::size_t hash_value_for(const T& x) noexcept {
  if constexpr (requires(std::hash<T> hasher) { hasher(x); }) {
    return std::hash<T>{}(x);
  } else if constexpr (requires { hash_value(x); }) {
    return hash_value(x);
  }
#if YK_UTIL_INCLUDE_BOOST
  else if constexpr (requires { ::yk::hash_range(x); }) {
    return ::yk::hash_range(x);
  }
#endif
  else {
    static_assert(false, "no hasher found");
  }
}

}  // namespace yk

#endif  // YK_HASH_HASH_VALUE_FOR_HPP
