#ifndef YK_CONCURRENT_POOL_TYPES_HPP
#define YK_CONCURRENT_POOL_TYPES_HPP

#include <type_traits>

#include <cstddef>

namespace yk {

namespace detail {
using concurrent_pool_size_type = std::make_signed_t<std::size_t>;
} // detail


struct [[nodiscard]] concurrent_pool_size_info
{
  detail::concurrent_pool_size_type size = 0, capacity = 0;
};

struct [[nodiscard]] concurrent_pool_access_result
{
  bool ok = false;
  detail::concurrent_pool_size_type size = 0;

  [[nodiscard]]
  constexpr explicit operator bool() const noexcept { return ok; }
};

} // yk

#endif
