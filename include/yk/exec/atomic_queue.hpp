#ifndef YK_EXEC_ATOMIC_QUEUE_HPP
#define YK_EXEC_ATOMIC_QUEUE_HPP

#include "yk/exec/queue_traits.hpp"

#include "yk/arch.hpp"
#include "yk/no_unique_address.hpp"

#include <version>

#if __cpp_lib_jthread >= 201911L
#include <stop_token>
#endif

#include <algorithm> // min, max
#include <utility>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include <cstddef>

namespace yk::exec {

namespace detail {

template <class T, class Alloc>
struct atomic_queue_slot
{
  explicit atomic_queue_slot(const Alloc& allocator = {}) noexcept
    : allocator_(allocator)
  {
    if constexpr (alignof(T) < yk::hardware_destructive_interference_size) {
      constexpr std::size_t storage_ofs = std::max(alignof(T), sizeof(turn));
      static_assert(offsetof(atomic_queue_slot, storage) == storage_ofs);

      constexpr std::size_t overlapped = yk::hardware_destructive_interference_size - storage_ofs;
      static_assert(
        sizeof(atomic_queue_slot) ==
        yk::hardware_destructive_interference_size
          * (1 + (sizeof(T) - overlapped + yk::hardware_destructive_interference_size - 1) / yk::hardware_destructive_interference_size)
      );

    } else if constexpr (alignof(T) == yk::hardware_destructive_interference_size) {
      static_assert(offsetof(atomic_queue_slot, storage) == yk::hardware_destructive_interference_size);

      static_assert(
        sizeof(atomic_queue_slot) ==
        yk::hardware_destructive_interference_size
          * (1 + (sizeof(T) + yk::hardware_destructive_interference_size - 1) / yk::hardware_destructive_interference_size)
      );

    } else {
      static_assert(offsetof(atomic_queue_slot, storage) == alignof(T));

      static_assert(
        sizeof(atomic_queue_slot) ==
        alignof(T) * (1 + (sizeof(T) + alignof(T) - 1) / alignof(T))
      );
    }
  }

  ~atomic_queue_slot() noexcept(std::is_nothrow_destructible_v<T>)
  {
    if (turn & 1) destroy();
  }

  template <class... Args>
  void construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    std::allocator_traits<Alloc>::construct(
      allocator_, /*std::launder(*/ reinterpret_cast<T*>(&storage) /*)*/, std::forward<Args>(args)...
    );
  }

  void destroy() noexcept(std::is_nothrow_destructible_v<T>)
  {
    std::allocator_traits<Alloc>::destroy(
      allocator_, std::launder(reinterpret_cast<T*>(&storage))
    );
  }

  [[nodiscard]]
  T&& extract() noexcept
  {
    return static_cast<T&&>(*std::launder(reinterpret_cast<T*>(&storage)));
  }

YK_FORCEALIGN_BEGIN
  alignas(yk::hardware_destructive_interference_size) std::atomic<std::size_t> turn = 0;

  // Permit true sharing if alignas(T) is smaller than alignof(turn).
  // Our benchmark show this does not degrade the performance.
  alignas(T) std::byte storage[sizeof(T)];
YK_FORCEALIGN_END

  // This MUST be placed at the end, see: https://developercommunity.visualstudio.com/t/msvc::no_unique_address-leads-to-ext/10898323
  YK_NO_UNIQUE_ADDRESS Alloc allocator_;
};


template <class T, class Alloc>
struct atomic_queue_store_dynamic
{
private:
  using slot_type = atomic_queue_slot<T, Alloc>;
  using slot_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<slot_type>;

public:
  using value_type = T;
  using allocator_type = Alloc;
  using size_type = std::size_t;

  explicit atomic_queue_store_dynamic(const size_type capacity, const Alloc& allocator = {})
    : capacity_(capacity)
    , slot_allocator_(allocator)
  {
    static_assert(sizeof(atomic_queue_store_dynamic) == sizeof(capacity_) + sizeof(slots_));

    if (capacity_ < 1) {
      throw std::bad_alloc{};
    }

    slots_ = std::allocator_traits<slot_allocator_type>::allocate(
      slot_allocator_, capacity_ + 1
    );

    for (size_type i = 0; i < capacity_; ++i) {
      std::allocator_traits<slot_allocator_type>::construct(slot_allocator_, &slots_[i]);
    }
  }

  ~atomic_queue_store_dynamic()
  {
    for (size_type i = 0; i < capacity_; ++i) {
      std::allocator_traits<slot_allocator_type>::destroy(
        slot_allocator_, &slots_[i]
      );
    }
    std::allocator_traits<slot_allocator_type>::deallocate(
      slot_allocator_, slots_, capacity_ + 1
    );
  }

  [[nodiscard]]
  size_type capacity() const noexcept { return capacity_; }

  [[nodiscard]]
  slot_type& slot_at(std::size_t i) noexcept
  {
    return slots_[i];
  }

private:
  size_type capacity_;
  slot_type* slots_;

  // This MUST be placed at the end, see: https://developercommunity.visualstudio.com/t/msvc::no_unique_address-leads-to-ext/10898323
  YK_NO_UNIQUE_ADDRESS slot_allocator_type slot_allocator_;
};

template <class T, std::size_t N, class Alloc>
struct atomic_queue_store_static
{
private:
  static_assert(N >= 1);
  using slot_type = atomic_queue_slot<T, Alloc>;
  using slot_allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<slot_type>;

public:
  using value_type = T;
  using allocator_type = Alloc;
  using size_type = std::size_t;

  explicit atomic_queue_store_static(const Alloc& allocator = {}) noexcept
    : slot_allocator_{allocator}
  {
    static_assert(sizeof(atomic_queue_store_static) == alignof(slot_type) * ((sizeof(slots_) + alignof(slot_type) - 1) / alignof(slot_type)));

    for (size_type i = 0; i < N; ++i) {
      std::allocator_traits<slot_allocator_type>::construct(
        slot_allocator_, /*std::launder(*/ reinterpret_cast<slot_type*>(&slots_[sizeof(slot_type) * i]) /*)*/
      );
    }
  }

  // For compatibility; static queue with dynamic capacity makes sense only if capacity <= N
  explicit atomic_queue_store_static(const size_type capacity, const Alloc& allocator = {})
    : slot_allocator_{allocator}
  {
    static_assert(sizeof(atomic_queue_store_static) == alignof(slot_type) * ((sizeof(slots_) + alignof(slot_type) - 1) / alignof(slot_type)));

    if (N < capacity) {
      throw std::bad_alloc{};
    }

    for (size_type i = 0; i < N; ++i) {
      std::allocator_traits<slot_allocator_type>::construct(
        slot_allocator_, /*std::launder(*/ reinterpret_cast<slot_type*>(&slots_[sizeof(slot_type) * i]) /*)*/
      );
    }
  }

  ~atomic_queue_store_static()
  {
    for (size_type i = 0; i < N; ++i) {
      std::allocator_traits<slot_allocator_type>::destroy(
        slot_allocator_,
        std::launder(reinterpret_cast<slot_type*>(&slots_[sizeof(slot_type) * i]))
      );
    }
  }

  [[nodiscard]]
  constexpr size_type capacity() const noexcept { return N; }

  [[nodiscard]]
  slot_type& slot_at(std::size_t i) noexcept
  {
    return *std::launder(reinterpret_cast<slot_type*>(&slots_[sizeof(slot_type) * i]));
  }

private:
YK_FORCEALIGN_BEGIN
  alignas(slot_type) std::byte slots_[sizeof(slot_type) * (N + 1)];
YK_FORCEALIGN_END

  // This MUST be placed at the end, see: https://developercommunity.visualstudio.com/t/msvc::no_unique_address-leads-to-ext/10898323
  YK_NO_UNIQUE_ADDRESS slot_allocator_type slot_allocator_;
};

template <class StoreT, class T, class Alloc>
class atomic_queue_impl
{
  using store_type = StoreT;

public:
  using value_type = T;
  using allocator_type = Alloc;
  using size_type = std::size_t;

  explicit atomic_queue_impl(size_type capacity, const Alloc& allocator = {}) /* noexcept */
    : store_(capacity, allocator)
  {}

  explicit atomic_queue_impl(const Alloc& allocator = {})
    noexcept(std::is_nothrow_constructible_v<StoreT, const Alloc&>)
    requires std::constructible_from<StoreT, const Alloc&>
    : store_(allocator)
  {}

  atomic_queue_impl(const atomic_queue_impl&) = delete;
  atomic_queue_impl(atomic_queue_impl&&) = delete;
  atomic_queue_impl& operator=(const atomic_queue_impl&) = delete;
  atomic_queue_impl& operator=(atomic_queue_impl&&) = delete;

  // --------------------------------

  template <class... Args>
  void push(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    const auto head = head_.fetch_add(1);
    auto& slot = store_.slot_at(idx(head));

    while (turn(head) * 2 != slot.turn.load(std::memory_order_acquire));

    slot.construct(std::forward<Args>(args)...);
    slot.turn.store(turn(head) * 2 + 1, std::memory_order_release);
  }

  template <class... Args>
  [[nodiscard]]
  bool try_push(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    auto head = head_.load(std::memory_order_acquire);

    while (true) {
      auto& slot = store_.slot_at(idx(head));

      if (turn(head) * 2 == slot.turn.load(std::memory_order_acquire)) {
        if (head_.compare_exchange_strong(head, head + 1)) {
          slot.construct(std::forward<Args>(args)...);
          slot.turn.store(turn(head) * 2 + 1, std::memory_order_release);
          return true;
        }

      } else {
        const auto prev_head = head;
        head = head_.load(std::memory_order_acquire);
        if (head == prev_head) return false;
      }
    }
  }

  void pop(T& v) noexcept(std::is_nothrow_destructible_v<T>)
  {
    const auto tail = tail_.fetch_add(1);
    auto& slot = store_.slot_at(idx(tail));

    while (turn(tail) * 2 + 1 != slot.turn.load(std::memory_order_acquire));

    v = slot.extract();
    slot.destroy();
    slot.turn.store(turn(tail) * 2 + 2, std::memory_order_release);
  }

  [[nodiscard]]
  bool try_pop(T& v) noexcept(std::is_nothrow_destructible_v<T>)
  {
    auto tail = tail_.load(std::memory_order_acquire);

    while (true) {
      auto& slot = store_.slot_at(idx(tail));

      if (turn(tail) * 2 + 1 == slot.turn.load(std::memory_order_acquire)) {
        if (tail_.compare_exchange_strong(tail, tail + 1)) {
          v = slot.extract();
          slot.destroy();
          slot.turn.store(turn(tail) * 2 + 2, std::memory_order_release);
          return true;
        }

      } else {
        const auto prev_tail = tail;
        tail = tail_.load(std::memory_order_acquire);
        if (tail == prev_tail) return false;
      }
    }
  }

  [[nodiscard]]
  size_type capacity() const noexcept { return store_.capacity(); }

  [[nodiscard]]
  size_type size() const noexcept
  {
    const auto diff = static_cast<std::ptrdiff_t>(head_.load(std::memory_order_relaxed) - tail_.load(std::memory_order_relaxed));
    return static_cast<size_type>(diff < 0 ? -diff : diff);
  }

private:
  [[nodiscard]] std::size_t idx(std::size_t i)  const noexcept { return i % this->capacity(); }
  [[nodiscard]] std::size_t turn(std::size_t i) const noexcept { return i / this->capacity(); }

YK_FORCEALIGN_BEGIN
  alignas(yk::hardware_destructive_interference_size) std::atomic<std::size_t> head_ = 0;
  alignas(yk::hardware_destructive_interference_size) std::atomic<std::size_t> tail_ = 0;

  alignas(std::max(yk::hardware_destructive_interference_size, alignof(store_type)))
  store_type store_;
YK_FORCEALIGN_END
};

} // detail


template <class T, class Alloc = std::allocator<T>>
class atomic_queue : public detail::atomic_queue_impl<detail::atomic_queue_store_dynamic<T, Alloc>, T, Alloc>
{
public:
  using atomic_queue::atomic_queue_impl::atomic_queue_impl;
};

template <class T, std::size_t N, class Alloc = std::allocator<T>>
class static_atomic_queue : public detail::atomic_queue_impl<detail::atomic_queue_store_static<T, N, Alloc>, T, Alloc>
{
public:
  using static_atomic_queue::atomic_queue_impl::atomic_queue_impl;
};


// ------------------------------------------

#if __cpp_lib_jthread >= 201911L

template <class T, class Alloc>
struct queue_traits<atomic_queue<T, Alloc>>
{
  using queue_type = atomic_queue<T, Alloc>;
  using value_type = T;

  static constexpr bool need_stop_token_for_cancel = true;

  template <class... Args>
  [[nodiscard]]
  static bool cancelable_bounded_push(std::stop_token const& stop_token, queue_type& queue, Args&&... args)
  {
    while (!stop_token.stop_requested()) {
      if (queue.try_push(std::forward<Args>(args)...)) return true;
    }
    return false;
  }

  [[nodiscard]]
  static bool cancelable_pop(std::stop_token const& stop_token, queue_type& queue, T& value)
  {
    while (!stop_token.stop_requested()) {
      if (queue.try_pop(value)) return true;
    }
    return false;
  }
};

template <class T, std::size_t N, class Alloc>
struct queue_traits<static_atomic_queue<T, N, Alloc>>
{
  using queue_type = static_atomic_queue<T, N, Alloc>;
  using value_type = T;

  static constexpr bool need_stop_token_for_cancel = true;

  template <class... Args>
  [[nodiscard]]
  static bool cancelable_bounded_push(std::stop_token const& stop_token, queue_type& queue, Args&&... args)
  {
    while (!stop_token.stop_requested()) {
      if (queue.try_push(std::forward<Args>(args)...)) return true;
    }
    return false;
  }

  [[nodiscard]]
  static bool cancelable_pop(std::stop_token const& stop_token, queue_type& queue, T& value)
  {
    while (!stop_token.stop_requested()) {
      if (queue.try_pop(value)) return true;
    }
    return false;
  }
};

#endif // stop_token

} // yk::exec

#endif
