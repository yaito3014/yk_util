#ifndef YK_HASH_RANGE_HPP
#define YK_HASH_RANGE_HPP

#include <boost/container_hash/hash_fwd.hpp>

#include <ranges>
#include <utility>

#include <cassert>
#include <cstddef>

namespace yk {

template <std::ranges::range R>
[[nodiscard]] constexpr std::size_t hash_range(R&& r) noexcept {
  std::size_t seed = 0;
  for (auto&& elem : r) boost::hash_combine(seed, elem);
  return seed;
}

}  // namespace yk

#endif  // YK_HASH_RANGE_HPP
