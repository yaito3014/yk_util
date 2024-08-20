﻿#ifndef YK_ALLOCATOR_DEFAULT_INIT_ALLOCATOR_HPP
#define YK_ALLOCATOR_DEFAULT_INIT_ALLOCATOR_HPP

#include "yk/allocator/concepts.hpp"

#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace yk {

// http://stackoverflow.com/a/21028912/273767

template <class T, class A = std::allocator<T>>
class default_init_allocator : public A {
  static_assert(xo::simple_allocator<A>);

  using traits = std::allocator_traits<A>;

public:
  template <class U>
  struct rebind {
    using other = default_init_allocator<U, typename traits::template rebind_alloc<U>>;
  };

  using A::A;

  template <class U>
  constexpr void construct(U* ptr) noexcept(std::is_nothrow_default_constructible_v<U>) {
    ::new (static_cast<void*>(ptr)) U;
  }
  template <class U, class... Args>
  constexpr void construct(U* ptr, Args&&... args) {
    traits::construct(static_cast<A&>(*this), ptr, std::forward<Args>(args)...);
  }
};

}  // namespace yk

#endif
