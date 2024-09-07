#ifndef YK_HASH_HASH_COMBINE_HPP
#define YK_HASH_HASH_COMBINE_HPP

#include "yk/hash/hash_value_for/fwd.hpp"

#include <boost/container_hash/hash.hpp>

#include <cstddef>

namespace yk {

template <class... Ts>
[[nodiscard]] constexpr std::size_t hash_combine(const Ts&... xs) noexcept /* strengthened */ {
  std::size_t seed = 0;
  (boost::hash_combine(seed, ::yk::hash_value_for(xs)), ...);
  return seed;
}

}  // namespace yk

#endif  // YK_HASH_HASH_COMBINE_HPP
