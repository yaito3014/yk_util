#ifndef YK_FLAT_SET_HPP
#define YK_FLAT_SET_HPP

#include "yk/no_unique_address.hpp"

#include <algorithm>
#include <compare>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

namespace yk {

namespace xo {

template <class T>
concept boolean_testable_impl = std::convertible_to<T, bool>;

template <class T>
concept boolean_testable = boolean_testable_impl<T> && requires(T&& x) {
  { !std::forward<T>(x) } -> boolean_testable_impl;
};

template <class R, class T>
concept container_compatible_range =
    std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T>;

constexpr auto synth_three_way = []<class T, class U>(const T& t, const U& u)
  requires requires {
    { t < u } -> boolean_testable;
    { u < t } -> boolean_testable;
  }
{
  if constexpr (std::three_way_comparable_with<T, U>) {
    return t <=> u;
  } else {
    if (t < u) return std::weak_ordering::less;
    if (u < t) return std::weak_ordering::greater;
    return std::weak_ordering::equivalent;
  }
};

template <class T, class U = T>
using synth_three_way_result = decltype(synth_three_way(std::declval<T&>(), std::declval<U&>()));

template <class InputIterator>
using iter_value_type = typename std::iterator_traits<InputIterator>::value_type;

template <class Allocator, class T>
using alloc_rebind = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;

}  // namespace xo

struct sorted_unique_t {};
inline constexpr sorted_unique_t sorted_unique;

template <class Key, class Compare = std::less<Key>, class KeyContainer = std::vector<Key>>
class flat_set {
private:
  constexpr void sort_and_unique()
  {
    std::ranges::sort(cont_, compare_);
    const auto res = std::ranges::unique(cont_);
    cont_.erase(res.begin(), res.end());
  }

public:
  using key_type = Key;
  using value_type = Key;
  using key_compare = Compare;
  using value_compare = Compare;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = typename KeyContainer::size_type;
  using difference_type = typename KeyContainer::difference_type;
  using iterator = typename KeyContainer::const_iterator;
  using const_iterator = typename KeyContainer::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using container_type = KeyContainer;

  constexpr flat_set() : flat_set(Compare{}) {}

  constexpr explicit flat_set(const key_compare& compare) : compare_(compare), cont_() {}

  constexpr explicit flat_set(container_type container, const key_compare& compare = key_compare{})
      : compare_(compare), cont_(std::move(container))
  {
    sort_and_unique();
  }

  constexpr flat_set(sorted_unique_t, container_type container, const key_compare& compare = key_compare{})
      : compare_(compare), cont_(std::move(container))
  {
  }

  template <class InputIterator>
  constexpr flat_set(InputIterator first, InputIterator last, const key_compare& compare = key_compare{})
      : compare_(compare), cont_()
  {
    insert(first, last);
  }

  template <class InputIterator>
  constexpr flat_set(
      sorted_unique_t, InputIterator first, InputIterator last, const key_compare& compare = key_compare{}
  )
      : compare_(compare), cont_(first, last)
  {
  }

  template <xo::container_compatible_range<value_type> R>
  constexpr flat_set(std::from_range_t, R&& rg) : flat_set(std::from_range, std::forward<R>(rg), key_compare{})
  {
  }

  template <xo::container_compatible_range<value_type> R>
  constexpr flat_set(std::from_range_t, R&& rg, const key_compare& compare) : flat_set(compare)
  {
    insert_range(std::forward<R>(rg));
  }

  constexpr flat_set(std::initializer_list<value_type> il, const key_compare& compare = key_compare{})
      : flat_set(il.begin(), il.end(), compare)
  {
  }

  template <class Alloc>
  constexpr explicit flat_set(const Alloc& a) : flat_set(key_compare{}, a)
  {
  }

  template <class Alloc>
  constexpr flat_set(const key_compare& comp, const Alloc& a)
      : compare_(comp), cont_(std::make_obj_using_allocator<container_type>(a))
  {
  }

  template <class Alloc>
  constexpr flat_set(const container_type& cont, const Alloc& a) : flat_set(cont, key_compare{}, a)
  {
  }

  template <class Alloc>
  constexpr flat_set(const container_type& cont, const key_compare& comp, const Alloc& a)
      : flat_set(std::make_obj_using_allocator<container_type>(a, cont), comp)
  {
  }

  template <class Alloc>
  constexpr flat_set(sorted_unique_t, const container_type& cont, const Alloc& a)
      : flat_set(sorted_unique, cont, key_compare{}, a)
  {
  }

  template <class Alloc>
  constexpr flat_set(sorted_unique_t, const container_type& cont, const key_compare& comp, const Alloc& a)
      : compare_(comp), cont_(std::make_obj_using_allocator<container_type>(a, cont))
  {
  }

  template <class Alloc>
  constexpr flat_set(const flat_set& other, const Alloc& a)
      : compare_(other.compare_), cont_(std::make_obj_using_allocator<container_type>(a, other.cont_))
  {
  }

  template <class Alloc>
  constexpr flat_set(flat_set&& other, const Alloc& a)
      : compare_(other.compare_), cont_(std::make_obj_using_allocator<container_type>(a, std::move(other).cont_))
  {
  }

  template <class InputIterator, class Alloc>
  constexpr flat_set(InputIterator first, InputIterator last, const Alloc& a) : flat_set(first, last, key_compare{}, a)
  {
  }

  template <class InputIterator, class Alloc>
  constexpr flat_set(InputIterator first, InputIterator last, const key_compare& comp, const Alloc& a)
      : flat_set(std::make_obj_using_allocator<container_type>(a, first, last), comp)
  {
  }

  template <class InputIterator, class Alloc>
  constexpr flat_set(sorted_unique_t, InputIterator first, InputIterator last, const Alloc& a)
      : flat_set(sorted_unique, first, last, key_compare{}, a)
  {
  }

  template <class InputIterator, class Alloc>
  constexpr flat_set(sorted_unique_t, InputIterator first, InputIterator last, const key_compare& comp, const Alloc& a)
      : flat_set(sorted_unique, std::make_obj_using_allocator<container_type>(a, first, last), comp)
  {
  }

  template <xo::container_compatible_range<value_type> R, class Alloc>
  constexpr flat_set(std::from_range_t, R&& rg, const Alloc& a)
      : flat_set(std::from_range, std::forward<R>(rg), key_compare{}, a)
  {
  }

  template <xo::container_compatible_range<value_type> R, class Alloc>
  constexpr flat_set(std::from_range_t, R&& rg, const key_compare& comp, const Alloc& a)
      : flat_set(std::make_obj_using_allocator<container_type>(a, std::from_range, std::forward<R>(rg)), comp)
  {
  }

  template <class Alloc>
  constexpr flat_set(std::initializer_list<value_type> il, const Alloc& a) : flat_set(il, key_compare{}, a)
  {
  }

  template <class Alloc>
  constexpr flat_set(std::initializer_list<value_type> il, const key_compare& comp, const Alloc& a)
      : flat_set(std::make_obj_using_allocator<container_type>(a, il), comp)
  {
  }

  template <class Alloc>
  constexpr flat_set(sorted_unique_t, std::initializer_list<value_type> il, const Alloc& a)
      : flat_set(sorted_unique, il, key_compare{}, a)
  {
  }

  template <class Alloc>
  constexpr flat_set(sorted_unique_t, std::initializer_list<value_type> il, const key_compare& comp, const Alloc& a)
      : flat_set(sorted_unique, std::make_obj_using_allocator<container_type>(a, il), comp)
  {
  }

  constexpr flat_set& operator=(std::initializer_list<value_type> il)
  {
    // TODO: improve time complexity
    clear();
    insert(il);
  }

  constexpr iterator begin() noexcept { return cont_.begin(); }
  constexpr const_iterator begin() const noexcept { return cont_.begin(); }
  constexpr iterator end() noexcept { return cont_.end(); }
  constexpr const_iterator end() const noexcept { return cont_.end(); }

  constexpr reverse_iterator rbegin() noexcept { return cont_.rbegin(); }
  constexpr const_reverse_iterator rbegin() const noexcept { return cont_.rbegin(); }
  constexpr reverse_iterator rend() noexcept { return cont_.rend(); }
  constexpr const_reverse_iterator rend() const noexcept { return cont_.rend(); }

  constexpr const_iterator cbegin() const noexcept { return cont_.cbegin(); }
  constexpr const_iterator cend() const noexcept { return cont_.cend(); }
  constexpr const_reverse_iterator crbegin() const noexcept { return cont_.crbegin(); }
  constexpr const_reverse_iterator crend() const noexcept { return cont_.crend(); }

  constexpr bool empty() const noexcept { return cont_.empty(); }
  constexpr size_type size() const noexcept { return cont_.size(); }
  constexpr size_type max_size() const noexcept { return cont_.size(); }

  template <class... Args>
  constexpr std::pair<iterator, bool> emplace(Args&&... args);

  template <class... Args>
  constexpr iterator emplace_hint(const_iterator position, Args&&... args);

  constexpr std::pair<iterator, bool> insert(const value_type& x) { return emplace(x); }

  constexpr std::pair<iterator, bool> insert(value_type&& x) { return emplace(std::move(x)); }

  template <class K>
    requires std::is_constructible_v<value_type, K> && requires { typename Compare::is_transparent; }
  constexpr std::pair<iterator, bool> insert(K&& x);

  constexpr iterator insert(const_iterator position, const value_type& x) { return emplace_hint(position, x); }

  constexpr iterator insert(const_iterator position, value_type&& x) { return emplace_hint(position, std::move(x)); }

  template <class K>
    requires std::is_constructible_v<value_type, K> && requires { typename Compare::is_transparent; }
  constexpr iterator insert(const_iterator hint, K&& x);

  template <class InputIterator>
  constexpr void insert(InputIterator first, InputIterator last)
  {
    // TODO: improve time complexity
    cont_.insert(cont_.end(), first, last);
    sort_and_unique();
  }

  template <class InputIterator>
  constexpr void insert(sorted_unique_t, InputIterator first, InputIterator last)
  {
    // TODO: improve time complexity
    cont_.insert(cont_.end(), first, last);
    sort_and_unique();
  }

  template <xo::container_compatible_range<value_type> R>
  constexpr void insert_range(R&& rg)
  {
    // TODO: improve time complexity
    cont_.insert_range(std::forward<R>(rg));
    sort_and_unique();
  }

  constexpr void insert(std::initializer_list<value_type> il) { insert(il.begin(), il.end()); }
  constexpr void insert(sorted_unique_t s, std::initializer_list<value_type> il) { insert(s, il.begin(), il.end()); }

  constexpr container_type extract() && { return std::move(cont_); };
  constexpr void replace(container_type&& cont) { cont_ = std::move(cont); }

  constexpr iterator erase(const_iterator position) { return cont_.erase(position); }
  constexpr size_type erase(const key_type& x)
  {
    erase(lower_bound(x));
    return 1;
  }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr size_type erase(K&& x)
  {
    erase(lower_bound(x));
    return 1;
  }
  constexpr iterator erase(const_iterator first, const_iterator last) { return erase(first, last); }

  constexpr void swap(flat_set& y) noexcept
  {
    std::ranges::swap(compare_, y.compare_);
    std::ranges::swap(cont_, y.cont_);
  }
  constexpr void clear() noexcept { cont_.clear(); }

  constexpr key_compare key_comp() const { return compare_; }
  constexpr value_compare value_comp() const { return compare_; }

  constexpr iterator find(const key_type& x) { return lower_bound(x); }
  constexpr const_iterator find(const key_type& x) const { return lower_bound(x); }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr iterator find(const K& x)
  {
    return lower_bound(x);
  }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr const_iterator find(const K& x) const
  {
    return lower_bound(x);
  }

  constexpr size_type count(const key_type& x) const { return std::ranges::equal_range(cont_, x, compare_).size(); }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr size_type count(const K& x) const
  {
    return std::ranges::equal_range(cont_, x, compare_).size();
  }

  constexpr bool contains(const key_type& x) const { return std::ranges::contains(cont_, x); }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr bool contains(const K& x) const
  {
    return std::ranges::contains(cont_, x);
  }

  constexpr iterator lower_bound(const key_type& x) { return std::ranges::lower_bound(cont_, x, compare_); }
  constexpr const_iterator lower_bound(const key_type& x) const { return std::ranges::lower_bound(cont_, x, compare_); }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr iterator lower_bound(const K& x)
  {
    return std::ranges::lower_bound(cont_, x, compare_);
  }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr const_iterator lower_bound(const K& x) const
  {
    return std::ranges::lower_bound(cont_, x, compare_);
  }

  constexpr iterator upper_bound(const key_type& x) { return std::ranges::upper_bound(cont_, x, compare_); }
  constexpr const_iterator upper_bound(const key_type& x) const { return std::ranges::upper_bound(cont_, x, compare_); }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr iterator upper_bound(const K& x)
  {
    return std::ranges::upper_bound(cont_, x, compare_);
  }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr const_iterator upper_bound(const K& x) const
  {
    return std::ranges::upper_bound(cont_, x, compare_);
  }

  constexpr std::pair<iterator, iterator> equal_range(const key_type& x)
  {
    return std::ranges::equal_range(cont_, x, compare_);
  }
  constexpr std::pair<const_iterator, const_iterator> equal_range(const key_type& x) const
  {
    return std::ranges::equal_range(cont_, x, compare_);
  }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr std::pair<iterator, iterator> equal_range(const K& x)
  {
    return std::ranges::equal_range(cont_, x, compare_);
  }
  template <class K>
    requires requires { typename Compare::is_transparent; }
  constexpr std::pair<const_iterator, const_iterator> equal_range(const K& x) const
  {
    return std::ranges::equal_range(cont_, x, compare_);
  }

  constexpr friend bool operator==(const flat_set& x, const flat_set& y) { return x.cont_ == y.cont_; }

  constexpr friend xo::synth_three_way_result<value_type> operator<=>(const flat_set& x, const flat_set& y)
  {
    return x.cont_ <=> y.cont_;
  }

  constexpr friend void swap(flat_set& x, flat_set& y) noexcept { x.swap(y); }

private:
  YK_NO_UNIQUE_ADDRESS Compare compare_;
  KeyContainer cont_;
};

template <class KeyContainer, class Compare = std::less<typename KeyContainer::value_type>>
flat_set(KeyContainer, Compare = Compare()) -> flat_set<typename KeyContainer::value_type, Compare, KeyContainer>;

template <class KeyContainer, class Allocator>
flat_set(KeyContainer, Allocator)
    -> flat_set<typename KeyContainer::value_type, std::less<typename KeyContainer::value_type>, KeyContainer>;

template <class KeyContainer, class Compare = std::less<typename KeyContainer::value_type>>
flat_set(sorted_unique_t, KeyContainer, Compare = Compare())
    -> flat_set<typename KeyContainer::value_type, Compare, KeyContainer>;

template <class KeyContainer, class Allocator>
flat_set(sorted_unique_t, KeyContainer, Allocator)
    -> flat_set<typename KeyContainer::value_type, std::less<typename KeyContainer::value_type>, KeyContainer>;

template <class KeyContainer, class Compare, class Allocator>
flat_set(sorted_unique_t, KeyContainer, Compare, Allocator)
    -> flat_set<typename KeyContainer::value_type, Compare, KeyContainer>;

template <class InputIterator, class Compare = std::less<xo::iter_value_type<InputIterator>>>
flat_set(InputIterator, InputIterator, Compare = Compare()) -> flat_set<xo::iter_value_type<InputIterator>, Compare>;

template <class InputIterator, class Compare = std::less<xo::iter_value_type<InputIterator>>>
flat_set(sorted_unique_t, InputIterator, InputIterator, Compare = Compare())
    -> flat_set<xo::iter_value_type<InputIterator>, Compare>;

template <
    std::ranges::input_range R, class Compare = std::less<std::ranges::range_value_t<R>>,
    class Allocator = std::allocator<std::ranges::range_value_t<R>>>
flat_set(std::from_range_t, R&&, Compare = Compare(), Allocator = Allocator()) -> flat_set<
    std::ranges::range_value_t<R>, Compare,
    std::vector<std::ranges::range_value_t<R>, xo::alloc_rebind<Allocator, std::ranges::range_value_t<R>>>>;

template <std::ranges::input_range R, class Allocator>
flat_set(std::from_range_t, R&&, Allocator) -> flat_set<
    std::ranges::range_value_t<R>, std::less<std::ranges::range_value_t<R>>,
    std::vector<std::ranges::range_value_t<R>, xo::alloc_rebind<Allocator, std::ranges::range_value_t<R>>>>;

template <class Key, class Compare = std::less<Key>>
flat_set(std::initializer_list<Key>, Compare = Compare()) -> flat_set<Key, Compare>;

template <class Key, class Compare = std::less<Key>>
flat_set(sorted_unique_t, std::initializer_list<Key>, Compare = Compare()) -> flat_set<Key, Compare>;

}  // namespace yk

template <class Key, class Compare, class KeyContainer, class Allocator>
struct std::uses_allocator<yk::flat_set<Key, Compare, KeyContainer>, Allocator>
    : std::bool_constant<std::uses_allocator_v<KeyContainer, Allocator>> {};

#endif  // YK_FLAT_SET_HPP
