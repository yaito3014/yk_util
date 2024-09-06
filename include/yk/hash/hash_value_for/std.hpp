#ifndef YK_HASH_HASH_VALUE_FOR_STD_HPP
#define YK_HASH_HASH_VALUE_FOR_STD_HPP

#include <functional>

namespace yk {

template <class T>
[[nodiscard]] constexpr std::size_t std_hash_value_for(const T& x) noexcept {
  return std::hash<T>{}(x);
}

}  // namespace yk

#endif  // YK_HASH_HASH_VALUE_FOR_STD_HPP
