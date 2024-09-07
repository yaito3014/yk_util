#ifndef YK_HASH_PROXY_HASH_HPP
#define YK_HASH_PROXY_HASH_HPP

#include <functional>

#include <cstddef>

namespace yk {

template <class T, auto Proj = std::identity{}>
struct proxy_hash {
  using is_transparent = void;

  using projected_type = std::remove_cvref_t<std::invoke_result_t<decltype(Proj), T>>;

  [[nodiscard]] inline std::size_t operator()(const T& value) const noexcept { return std::hash<projected_type>{}(std::invoke(Proj, value)); }

  [[nodiscard]] inline std::size_t operator()(const projected_type& value) const noexcept { return std::hash<projected_type>{}(value); }
};

}  // namespace yk

#endif  // YK_HASH_PROXY_HASH_HPP
