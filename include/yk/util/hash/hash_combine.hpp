#ifndef YK_UTIL_HASH_HASH_COMBINE_HPP
#define YK_UTIL_HASH_HASH_COMBINE_HPP

#include <boost/container_hash/hash.hpp>

#include <cstddef>

namespace yk {

template <class T, class... Rest>
[[nodiscard]] constexpr std::size_t hash_combine(const T& first_arg, const Rest&... rest) noexcept /* strengthened */ {
  std::size_t seed = ::yk::hash_value_for(first_arg);
  (boost::hash_combine(seed, ::yk::hash_value_for(rest)), ...);
  return seed;
}

}  // namespace yk

#endif  // YK_HASH_HASH_COMBINE_HPP
