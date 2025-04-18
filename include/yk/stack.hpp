﻿#ifndef YK_STACK_HPP
#define YK_STACK_HPP

#include "yk/allocator/concepts.hpp"

#include <compare>
#include <iterator>
#include <memory>
#include <ranges>
#include <stack>
#include <type_traits>
#include <vector>
#include <version>

namespace yk {

// stack with clear()
// also, defaults to std::vector<T> instead of std::deque<T>
template <class T, class Container = std::vector<T>>
class stack : protected std::stack<T, Container> {
  using base_stack = std::stack<T, Container>;

public:
  using typename base_stack::const_reference;
  using typename base_stack::container_type;
  using typename base_stack::reference;
  using typename base_stack::size_type;
  using typename base_stack::value_type;

  using base_stack::base_stack;

  using base_stack::empty;
  using base_stack::push;
  using base_stack::size;
  using base_stack::top;

#if __cpp_lib_containers_ranges >= 202202L
  using base_stack::push_range;
#endif

  using base_stack::emplace;
  using base_stack::pop;

  constexpr void swap(stack& other) noexcept(std::is_nothrow_swappable_v<Container>) {
    using std::swap;
    swap(c, other.c);
  }

  // our addition
  constexpr stack(std::initializer_list<T> il) noexcept(noexcept(stack(Container(il.begin(), il.end())))) : stack(Container(il.begin(), il.end())) {}

  [[nodiscard]] constexpr size_type capacity() const noexcept
    requires requires(Container c) { c.capacity(); }
  {
    return c.capacity();
  }

  constexpr void clear() noexcept(noexcept(c.clear()))
    requires requires(Container c) { c.clear(); }
  {
    c.clear();
  }

  [[nodiscard]] constexpr size_type max_size() const noexcept { return c.max_size(); }

  constexpr void reserve(size_type n)
    requires requires(Container c) { c.reserve(n); }
  {
    c.reserve(n);
  }

  constexpr void shrink_to_fit()
    requires requires(Container c) { c.shrink_to_fit(); }
  {
    c.shrink_to_fit();
  }

protected:
  using base_stack::c;

public:
  [[nodiscard]] friend constexpr bool operator==(const stack& lhs, const stack& rhs) noexcept(noexcept(static_cast<const base_stack&>(lhs) ==
                                                                                                       static_cast<const base_stack&>(rhs))) {
    return static_cast<const base_stack&>(lhs) == static_cast<const base_stack&>(rhs);
  }

  [[nodiscard]] friend constexpr bool operator!=(const stack& lhs, const stack& rhs) noexcept(noexcept(static_cast<const base_stack&>(lhs) !=
                                                                                                       static_cast<const base_stack&>(rhs))) {
    return static_cast<const base_stack&>(lhs) != static_cast<const base_stack&>(rhs);
  }

  [[nodiscard]] friend constexpr bool operator<(const stack& lhs, const stack& rhs) noexcept(noexcept(static_cast<const base_stack&>(lhs) <
                                                                                                      static_cast<const base_stack&>(rhs))) {
    return static_cast<const base_stack&>(lhs) < static_cast<const base_stack&>(rhs);
  }

  [[nodiscard]] friend constexpr bool operator<=(const stack& lhs, const stack& rhs) noexcept(noexcept(static_cast<const base_stack&>(lhs) <=
                                                                                                       static_cast<const base_stack&>(rhs))) {
    return static_cast<const base_stack&>(lhs) <= static_cast<const base_stack&>(rhs);
  }

  [[nodiscard]] friend constexpr bool operator>(const stack& lhs, const stack& rhs) noexcept(noexcept(static_cast<const base_stack&>(lhs) >
                                                                                                      static_cast<const base_stack&>(rhs))) {
    return static_cast<const base_stack&>(lhs) > static_cast<const base_stack&>(rhs);
  }

  [[nodiscard]] friend constexpr bool operator>=(const stack& lhs, const stack& rhs) noexcept(noexcept(static_cast<const base_stack&>(lhs) >=
                                                                                                       static_cast<const base_stack&>(rhs))) {
    return static_cast<const base_stack&>(lhs) >= static_cast<const base_stack&>(rhs);
  }

  [[nodiscard]] friend constexpr auto operator<=>(const stack& lhs, const stack& rhs) noexcept(noexcept(static_cast<const base_stack&>(lhs) <=>
                                                                                                        static_cast<const base_stack&>(rhs)))
    requires std::three_way_comparable<Container>
  {
    return static_cast<const base_stack&>(lhs) <=> static_cast<const base_stack&>(rhs);
  }
};

template <class Container>
  requires (!xo::simple_allocator<Container>)
stack(Container) -> stack<typename Container::value_type, Container>;

template <class Container, class Alloc>
  requires (!xo::simple_allocator<Container>) && std::uses_allocator_v<Container, Alloc>
stack(Container, Alloc) -> stack<typename Container::value_type, Container>;

#if __cpp_lib_adaptor_iterator_pair_constructor >= 202106L
// C++23
template <std::input_iterator InputIt, xo::simple_allocator Alloc = std::allocator<std::iter_value_t<InputIt>>>
stack(InputIt, InputIt, Alloc = Alloc()) -> stack<std::iter_value_t<InputIt>, std::vector<std::iter_value_t<InputIt>, Alloc>>;
#endif

#if __cpp_lib_containers_ranges >= 202202L
// C++23
template <std::ranges::input_range R>
stack(std::from_range_t, R&&) -> stack<std::ranges::range_value_t<R>>;

// C++23
template <std::ranges::input_range R, xo::simple_allocator Alloc>
stack(std::from_range_t, R&&, Alloc) -> stack<std::ranges::range_value_t<R>, std::vector<std::ranges::range_value_t<R>, Alloc>>;
#endif

template <class T, class Container>
  requires std::is_swappable_v<Container>
void swap(stack<T, Container>& lhs, stack<T, Container>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
  lhs.swap(rhs);
}

}  // namespace yk

namespace std {

template <class T, class Container, class Alloc>
struct uses_allocator<::yk::stack<T, Container>, Alloc> : uses_allocator<Container, Alloc>::type {};

}  // namespace std

#endif
