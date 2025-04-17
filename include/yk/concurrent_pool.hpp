#ifndef YK_CONCURRENT_POOL_HPP
#define YK_CONCURRENT_POOL_HPP

#include "yk/allocator/default_init_allocator.hpp"

#include "yk/util/to_underlying.hpp"

#include "yk/interrupt_exception.hpp"

#include "yk/enum_bitops.hpp"

#include <concepts>
#include <condition_variable>
#include <mutex>
#include <stop_token>
#include <type_traits>

namespace yk {

enum struct concurrent_pool_flag : unsigned {
  multi_producer = 0b01,
  multi_consumer = 0b10,

  spsc = 0,                                // single-producer + single-consumer
  spmc = multi_consumer,                   // single-producer + multi-consumer
  mpsc = multi_producer,                   // multi-producer + single-consumer
  mpmc = multi_producer | multi_consumer,  // multi-producer + multi-consumer

  producer_consumer_mask = 0b11,

  stop_token_support = 0b100,

  queue_based_push_pop = 0b1000,
};

template <>
struct bitops_enabled<concurrent_pool_flag> : std::true_type {};

namespace detail {

template <class T, class PoolT, concurrent_pool_flag Flags>
struct concurrent_pool_traits {
  // clang-format off
  static constexpr concurrent_pool_flag flags   = Flags;
  static constexpr bool is_multi_producer       = static_cast<bool>(flags & concurrent_pool_flag::multi_producer);
  static constexpr bool is_single_producer      = !is_multi_producer;
  static constexpr bool is_multi_consumer       = static_cast<bool>(flags & concurrent_pool_flag::multi_consumer);
  static constexpr bool is_single_consumer      = !is_multi_consumer;
  static constexpr bool has_stop_token_support  = __cpp_lib_jthread >= 201911L && static_cast<bool>(flags & concurrent_pool_flag::stop_token_support);
  static constexpr bool is_queue_based_push_pop = static_cast<bool>(flags & concurrent_pool_flag::queue_based_push_pop);

  using condition_variable_type = std::conditional_t<
    has_stop_token_support,
    std::condition_variable_any, // has stop_token overload
    std::condition_variable
  >;
  // clang-format on

  static constexpr bool has_reserve = requires(PoolT pool) {
    { pool.reserve(std::size_t{}) };
  };

  static constexpr bool has_back_access = requires(T&& val, PoolT pool) {
    { pool.emplace_back(std::move(val)) };
    { pool.back() } -> std::convertible_to<T&>;
    { pool.pop_back() };
  };
  static constexpr bool has_front_access = requires(T&& val, PoolT pool) {
    { pool.emplace_front(std::move(val)) };
    { pool.front() } -> std::convertible_to<T&>;
    { pool.pop_front() };
  };
  static constexpr bool has_both_access = has_back_access && has_front_access;

  static constexpr bool has_plain_queue_access = requires(T&& val, PoolT pool) {
    { pool.front() } -> std::convertible_to<T&>;
    { pool.back() } -> std::convertible_to<T&>;
    { pool.emplace(std::move(val)) };
    { pool.pop() };
  };
  static constexpr bool has_plain_stack_access = requires(T&& val, PoolT pool) {
    { pool.top() } -> std::convertible_to<T&>;
    { pool.emplace(std::move(val)) };
    { pool.pop() };
  };

  static_assert(is_queue_based_push_pop ? (has_both_access || has_plain_queue_access) : true,
                "when queue_based_push_pop is set, the underlying container must support queue-like operations");

  static_assert(has_back_access || has_plain_stack_access, "the underlying container must support at least stack-like operations");

  static constexpr bool default_access_strategy_is_stack = !is_queue_based_push_pop;

  template <class U>
  static void push(PoolT& pool, U&& value, condition_variable_type& cv_not_empty)
    requires is_single_producer && is_single_consumer
  {
    const bool was_empty = pool.empty();
    do_push(pool, std::forward<U>(value));

    if (was_empty) {
      cv_not_empty.notify_one();
    }
  }

  template <class U>
  static void push(PoolT& pool, U&& value, condition_variable_type& cv_not_empty)
    requires (!(is_single_producer && is_single_consumer))
  {
    do_push(pool, std::forward<U>(value));

    if constexpr (is_single_consumer) {
      cv_not_empty.notify_one();
    } else {
      cv_not_empty.notify_all();
    }
  }

  static void pop(PoolT& pool, T& value, condition_variable_type& cv_not_full, long long pool_capacity)
    requires is_single_producer && is_single_consumer
  {
    const bool was_full = static_cast<long long>(pool.size()) >= pool_capacity;
    do_pop(pool, value);

    if (was_full) {
      cv_not_full.notify_one();
    }
  }

  static void pop(PoolT& pool, T& value, condition_variable_type& cv_not_full, long long pool_capacity)
    requires (!(is_single_producer && is_single_consumer))
  {
    const bool was_full = static_cast<long long>(pool.size()) >= pool_capacity;
    do_pop(pool, value);

    if (was_full) {
      if constexpr (is_single_producer) {
        cv_not_full.notify_one();
      } else {
        cv_not_full.notify_all();
      }
    }
  }

private:
  template <class U>
  static void do_push(PoolT& pool, U&& value) {
    if constexpr (has_back_access) {
      pool.emplace_back(std::forward<U>(value));

    } else {
      pool.emplace(std::forward<U>(value));
    }
  }

  static void do_pop(PoolT& pool, T& value) {
    if constexpr (default_access_strategy_is_stack) {
      if constexpr (has_back_access) {
        value = std::move(pool.back());
        pool.pop_back();

      } else {
        value = std::move(pool.top());
        pool.pop();
      }

    } else {  // queue-like access
      if constexpr (has_front_access) {
        value = std::move(pool.front());
        pool.pop_front();

      } else {
        value = std::move(pool.front());
        pool.pop();
      }
    }
  }
};

}  // namespace detail

struct concurrent_pool_size_info {
  long long size = 0, capacity = 0;
};

// clang-format off
template <class T>
using concurrent_pool_allocator_t = std::conditional_t<
    std::is_trivially_copyable_v<T>,
    yk::default_init_allocator<T>,
    std::allocator<T>
>;
// clang-format on

template <class T, class PoolT, concurrent_pool_flag Flags = concurrent_pool_flag::mpmc>
class concurrent_pool {
public:
  // clang-format off
  static constexpr concurrent_pool_flag flags = Flags;
  using value_type                            = T;
  using pool_type                             = PoolT;
  using traits_type                           = detail::concurrent_pool_traits<T, PoolT, Flags>;
  using condition_variable_type               = typename traits_type::condition_variable_type;
  // clang-format on

  // -------------------------------------------

  [[nodiscard]]
  long long capacity() const {
    std::unique_lock lock{mtx_};
    return capacity_;
  }

  void set_capacity(long long new_capacity) {
    std::unique_lock lock{mtx_};
    capacity_ = new_capacity == 0 ? 1 : new_capacity;
  }

  void reserve(long long new_capacity)
    requires traits_type::has_reserve
  {
    std::unique_lock lock{mtx_};
    capacity_ = new_capacity == 0 ? 1 : new_capacity;
    pool_.reserve(static_cast<std::size_t>(capacity_));
  }

  [[nodiscard]]
  concurrent_pool_size_info size_info() const {
    std::unique_lock lock{mtx_};
    return {.size = static_cast<long long>(pool_.size()), .capacity = capacity_};
  }

  // -------------------------------------------

  template <class U>
  bool push_wait(U&& value) {
    std::unique_lock lock{mtx_};
    cv_not_full_.wait(lock, push_wait_cond());
    if (push_wait_cond_error()) {
      return false;
    }

    traits_type::push(pool_, std::forward<U>(value), cv_not_empty_);
    return true;
  }

#if __cpp_lib_jthread >= 201911L
  template <class U>
  bool push_wait(U&& value, std::stop_token stop_token)
    requires (!traits_type::has_stop_token_support)
  = delete;

  template <class U>
  bool push_wait(U&& value, std::stop_token stop_token)
    requires traits_type::has_stop_token_support
  {
    std::unique_lock lock{mtx_};
    cv_not_full_.wait(lock, stop_token, push_wait_cond());
    if (stop_token.stop_requested()) {
      throw interrupt_exception{};
    }
    if (push_wait_cond_error()) {
      return false;
    }
#endif

    traits_type::push(pool_, std::forward<U>(value), cv_not_empty_);
    return true;
  }

  // -------------------------------------------

  bool pop_wait(T& value) {
    std::unique_lock lock{mtx_};
    cv_not_empty_.wait(lock, pop_wait_cond());
    if (pop_wait_cond_error()) {
      return false;
    }

    traits_type::pop(pool_, value, cv_not_full_, capacity_);
    return true;
  }

#if __cpp_lib_jthread >= 201911L
  bool pop_wait(T& value, std::stop_token stop_token)
    requires (!traits_type::has_stop_token_support)
  = delete;

  bool pop_wait(T& value, std::stop_token stop_token)
    requires traits_type::has_stop_token_support
  {
    std::unique_lock lock{mtx_};
    cv_not_empty_.wait(lock, stop_token, pop_wait_cond());
    if (stop_token.stop_requested()) {
      throw interrupt_exception{};
    }
    if (pop_wait_cond_error()) {
      return false;
    }

    traits_type::pop(pool_, value, cv_not_full_);
    return true;
  }
#endif

  // -------------------------------------------

  void close() {
    std::unique_lock lock{mtx_};
    closed_ = true;
    cv_not_full_.notify_all();
    cv_not_empty_.notify_all();
  }

  void open() {
    std::unique_lock lock{mtx_};
    closed_ = false;
    cv_not_full_.notify_all();
    cv_not_empty_.notify_all();
  }

  void clear() {
    std::unique_lock lock{mtx_};
    pool_.clear();
    cv_not_full_.notify_all();
    // cv_not_empty_.notify_all();
  }

private:
  auto push_wait_cond() const {
    return [this] { return static_cast<long long>(pool_.size()) < capacity_ || closed_; };
  }
  bool push_wait_cond_error() const { return closed_; }

  auto pop_wait_cond() const {
    return [this] { return !pool_.empty() || closed_; };
  }
  bool pop_wait_cond_error() const { return closed_; }

  mutable std::mutex mtx_;
  pool_type pool_;
  long long capacity_ = 1024;

  condition_variable_type cv_not_full_, cv_not_empty_;

  bool closed_ = false;
};

}  // namespace yk

#endif
