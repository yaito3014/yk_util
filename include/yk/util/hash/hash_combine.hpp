#ifndef YK_UTIL_HASH_HASH_COMBINE_HPP
#define YK_UTIL_HASH_HASH_COMBINE_HPP

#include <boost/container_hash/hash.hpp>

#include <cstddef>

namespace yk {

template <class T, class... Rest>
[[nodiscard]] constexpr std::size_t hash_combine(T const& first_arg, Rest const&... rest) noexcept /* strengthened */
{
  std::size_t seed = hash_value(first_arg);
  (boost::hash_combine(seed, hash_value(rest)), ...);
  return seed;
}

}  // namespace yk

#endif  // YK_HASH_HASH_COMBINE_HPP
