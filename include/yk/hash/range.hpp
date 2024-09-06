#ifndef YK_HASH_RANGE_HPP
#define YK_HASH_RANGE_HPP

#include <boost/container_hash/hash_fwd.hpp>

#include <ranges>
#include <utility>

#include <cstddef>

namespace yk {

template <std::ranges::range R>
[[nodiscard]] constexpr std::size_t hash_range(R&& r) noexcept {
  auto common = std::views::common(std::forward<R>(r));
  return boost::hash_range(common.begin(), common.end());
}

}  // namespace yk

#endif  // YK_HASH_RANGE_HPP
