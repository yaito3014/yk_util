#ifndef YK_HASH_RANGE_HPP
#define YK_HASH_RANGE_HPP

#include "yk/hash/hash_combine.hpp"

#include <ranges>
#include <utility>

#include <cassert>
#include <cstddef>

namespace yk {

template <class T>
constexpr std::size_t hash_value_for(const T&) noexcept;

template <std::ranges::range R>
[[nodiscard]] constexpr std::size_t hash_range(R&& r) noexcept {
  assert(!std::ranges::empty(r));
  std::size_t seed = ::yk::hash_value_for(*std::ranges::begin(r));
  for (auto&& elem : r | std::views::drop(1)) seed = ::yk::hash_combine(seed, elem);
  return seed;
}

}  // namespace yk

#endif  // YK_HASH_RANGE_HPP
