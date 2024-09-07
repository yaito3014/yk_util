#ifndef YK_HASH_HASH_VALUE_FOR_HPP
#define YK_HASH_HASH_VALUE_FOR_HPP

#include "yk/hash/range.hpp"

#include <boost/container_hash/hash_fwd.hpp>

#include <functional>

namespace yk {

template <class T>
[[nodiscard]] inline std::size_t std_hash_value_for(const T& x) noexcept {
  return std::hash<T>{}(x);
}

template <class T>
[[nodiscard]] inline std::size_t boost_hash_value_for(const T& x) noexcept {
  return boost::hash<T>{}(x);
}

template <class T>
[[nodiscard]] inline std::size_t hash_value_for(const T& x) noexcept {
  if constexpr (requires(std::hash<T> hasher) { hasher(x); }) {
    return std::hash<T>{}(x);
  } else if constexpr (requires { hash_value(x); }) {
    return hash_value(x);
  } else if constexpr (requires { ::yk::hash_range(x); }) {
    return ::yk::hash_range(x);
  } else {
    static_assert(false, "no hasher found");
  }
}

}  // namespace yk

#endif  // YK_HASH_HASH_VALUE_FOR_HPP
