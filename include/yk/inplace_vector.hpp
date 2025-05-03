#ifndef YK_INPLACE_VECTOR_HPP
#define YK_INPLACE_VECTOR_HPP

// configurable
#ifndef YK_INPLACE_VECTOR_IS_ALLOCATOR_AWARE
# define YK_INPLACE_VECTOR_IS_ALLOCATOR_AWARE true
#endif

// std::inplace_vector https://en.cppreference.com/w/cpp/container/inplace_vector

// [P3160R0] An Allocator-aware inplace_vector https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3160r0.html
// [P3581R0] No, inplace_vector shouldn't have an Allocator https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3581r0.pdf
// cplusplus/papers: https://github.com/cplusplus/papers/issues/2224

// P3581R0:
// > We care about this use case.
// > It should be a different vocabulary type, such as basic_inplace_vector<T, N, A>.
//
// LEWG agreed that we 'care' about allocator support for inplace_vector.
// As noted above, it was just rejected because
//   (1) it should become an independent type, and
//   (2) it's too late for C++26.
//
// So we (yk_util) do have motivation to support this anyways.
// However, it turns out that the only decent implementation
// (sg14's "sg14/aa_inplace_vector.h" and "sg14/inplace_vector.h")
// has serious QoL issue that
//   - `#pragma once` is being used; effectively destroying preprocessor-level detection
//   - The original version and the allocator-aware version sits in the same namespace with same class names
//
// For these reasons, it's currently impossible to mix them.
// So here we provide a compatibility header, making the allocator-aware version default.
// (If I read correctly, sg14's allocator-aware version has zero overhead
//  when the default allocator is used)
//

#ifdef SG14_INPLACE_VECTOR_THROW // already included?

#include <memory>

namespace yk::detail {

template<class T, std::size_t N, class Alloc>
concept InplaceVectorIsAllocatorAware = requires {
    { ::sg14::inplace_vector<T, N, Alloc>{} };
};

// ODR violation check
static_assert(InplaceVectorIsAllocatorAware<int, 1, std::allocator<int>> == YK_INPLACE_VECTOR_IS_ALLOCATOR_AWARE);

} // yk::detail

#endif // already included?


#if YK_INPLACE_VECTOR_IS_ALLOCATOR_AWARE
#include "sg14/aa_inplace_vector.h"

namespace yk {

template <class T, std::size_t N, class Alloc = std::allocator<T>>
using inplace_vector = ::sg14::inplace_vector<T, N, Alloc>;

} // yk

#else
#include "sg14/inplace_vector.h"

namespace yk {

template <class T, std::size_t N>
using inplace_vector = ::sg14::inplace_vector<T, N>;

} // yk
#endif

#endif
