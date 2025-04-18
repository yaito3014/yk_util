#ifndef YK_ALLOCATOR_CONCEPTS_HPP
#define YK_ALLOCATOR_CONCEPTS_HPP

#include <concepts>

#include <cstddef>

namespace yk {

namespace xo {  // exposition-only

// C++26
// https://en.cppreference.com/w/cpp/named_req/Allocator
template <class Alloc>
concept simple_allocator = requires(Alloc alloc, std::size_t n) {
  { *alloc.allocate(n) } -> std::same_as<typename Alloc::value_type&>;
  { alloc.deallocate(alloc.allocate(n), n) };
} && std::copy_constructible<Alloc> && std::equality_comparable<Alloc>;

}  // namespace xo

}  // namespace yk

#endif
