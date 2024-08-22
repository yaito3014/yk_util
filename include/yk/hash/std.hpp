#ifndef YK_HASH_STD_HPP
#define YK_HASH_STD_HPP

#include "yk/hash/hash.hpp"

#include <functional>

namespace yk {

template <class T>
[[nodiscard]] constexpr std::size_t std_hash_value_for(const T& x) noexcept {
  return std::hash<T>{}(x);
}

template <class T>
[[nodiscard]] constexpr std::size_t hash_value_for(const T& x) noexcept {
  if constexpr (requires(std::hash<T> hasher) { hasher(x); }) {
    return std::hash<T>{}(x);
  } else {
    return hash_value(x);
  }
}

}  // namespace yk

#endif  // YK_HASH_STD_HPP
