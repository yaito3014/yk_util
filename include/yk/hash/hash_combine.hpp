#ifndef YK_UTIL_HASH_HASH_COMBINE_HPP
#define YK_UTIL_HASH_HASH_COMBINE_HPP

#include <boost/container_hash/hash_fwd.hpp>

#include <cstddef>

namespace yk {

template <class... Ts>
[[nodiscard]] constexpr std::size_t hash_combine(const Ts&... xs) noexcept /* strengthened */ {
  std::size_t seed = 0;
  (boost::hash_combine(seed, xs), ...);
  return seed;
}

}  // namespace yk

#endif  // YK_HASH_HASH_COMBINE_HPP
