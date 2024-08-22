#ifndef YK_UTIL_HASH_STD_HPP
#define YK_UTIL_HASH_STD_HPP

#include "yk/util/hash/hash.hpp"

#include <functional>

namespace yk {

template <class T>
[[nodiscard]] constexpr std::size_t hash_value_for(const T& x) noexcept {
  return std::hash<T>{}(x);
}

}  // namespace yk

#endif  // YK_UTIL_HASH_STD_HPP
