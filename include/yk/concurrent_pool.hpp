#ifndef YK_CONCURRENT_POOL_HPP
#define YK_CONCURRENT_POOL_HPP

#include "yk/allocator/default_init_allocator.hpp"
#include "yk/util/to_underlying.hpp"
#include "yk/enum_bitops.hpp"
#include "yk/interrupt_exception.hpp"

#include <concepts>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <stop_token>
#include <type_traits>
#include <utility>

#include <cstddef>

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

using concurrent_pool_size_type = std::make_signed_t<std::size_t>;

template <class T, class BufT, concurrent_pool_flag Flags>
struct concurrent_pool_traits {
  using value_type = T;
  using buf_type = BufT;
  using size_type = concurrent_pool_size_type;

  static constexpr concurrent_pool_flag flags   = Flags;
  static constexpr bool is_multi_producer       = static_cast<bool>(flags & concurrent_pool_flag::multi_producer);
  static constexpr bool is_single_producer      = !is_multi_producer;
  static constexpr bool is_multi_consumer       = static_cast<bool>(flags & concurrent_pool_flag::multi_consumer);
  static constexpr bool is_single_consumer      = !is_multi_consumer;
#if __cpp_lib_jthread >= 201911L
  static constexpr bool has_stop_token_support  = static_cast<bool>(flags & concurrent_pool_flag::stop_token_support);
#else
  static constexpr bool has_stop_token_support  = false;
#endif
  static constexpr bool is_queue_based_push_pop = static_cast<bool>(flags & concurrent_pool_flag::queue_based_push_pop);

  using condition_variable_type = std::conditional_t<
    has_stop_token_support,
    std::condition_variable_any, // has stop_token overload
    std::condition_variable
  >;

  static constexpr bool has_reserve = requires(BufT buf) {
    { buf.reserve(std::size_t{}) };
  };

  static constexpr bool has_back_access = requires(T&& val, BufT buf) {
    { buf.emplace_back(std::move(val)) };
    { buf.back() } -> std::convertible_to<T&>;
    { buf.pop_back() };
  };
  static constexpr bool has_front_access = requires(T&& val, BufT buf) {
    { buf.emplace_front(std::move(val)) };
    { buf.front() } -> std::convertible_to<T&>;
    { buf.pop_front() };
  };
  static constexpr bool has_both_access = has_back_access && has_front_access;

  static constexpr bool has_plain_queue_access = requires(T&& val, BufT buf) {
    { buf.front() } -> std::convertible_to<T&>;
    { buf.back() } -> std::convertible_to<T&>;
    { buf.emplace(std::move(val)) };
    { buf.pop() };
  };
  static constexpr bool has_plain_stack_access = requires(T&& val, BufT buf) {
    { buf.top() } -> std::convertible_to<T&>;
    { buf.emplace(std::move(val)) };
    { buf.pop() };
  };

  static_assert(
    is_queue_based_push_pop ? (has_both_access || has_plain_queue_access) : true,
    "when queue_based_push_pop is set, the underlying container must support queue-like operations"
  );

  static_assert(
    has_back_access || has_plain_stack_access,
    "the underlying container must support at least stack-like operations"
  );

  static constexpr bool default_access_strategy_is_stack = !is_queue_based_push_pop;

  template <class U>
  static void push(BufT& buf, U&& value, condition_variable_type& cv_not_empty)
    requires is_single_producer && is_single_consumer
  {
    const bool was_empty = buf.empty();
    do_push(buf, std::forward<U>(value));

    if (was_empty) {
      cv_not_empty.notify_one();
    }
  }

  template <class U>
  static void push(BufT& buf, U&& value, condition_variable_type& cv_not_empty)
    requires (!(is_single_producer && is_single_consumer))
  {
    do_push(buf, std::forward<U>(value));

    if constexpr (is_single_consumer) {
      cv_not_empty.notify_one();
    } else {
      cv_not_empty.notify_all();
    }
  }

  static void pop(BufT& buf, T& value, condition_variable_type& cv_not_full, size_type capacity)
    requires is_single_producer && is_single_consumer
  {
    const bool was_full = static_cast<size_type>(buf.size()) >= capacity;
    do_pop(buf, value);

    if (was_full) {
      cv_not_full.notify_one();
    }
  }

  static void pop(BufT& buf, T& value, condition_variable_type& cv_not_full)
    requires (!(is_single_producer && is_single_consumer))
  {
    do_pop(buf, value);

    if constexpr (is_single_producer) {
      cv_not_full.notify_one();
    } else {
      cv_not_full.notify_all();
    }
  }

private:
  template <class U>
  static void do_push(BufT& buf, U&& value)
  {
    if constexpr (has_back_access) {
      buf.emplace_back(std::forward<U>(value));

    } else {
      buf.emplace(std::forward<U>(value));
    }
  }

  static void do_pop(BufT& buf, T& value)
  {
    if constexpr (default_access_strategy_is_stack) {
      if constexpr (has_back_access) {
        value = std::move(buf.back());
        buf.pop_back();

      } else {
        value = std::move(buf.top());
        buf.pop();
      }

    } else {  // queue-like access
      if constexpr (has_front_access) {
        value = std::move(buf.front());
        buf.pop_front();

      } else {
        value = std::move(buf.front());
        buf.pop();
      }
    }
  }
};

// implementation-defined
struct concurrent_pool_size_info {
  concurrent_pool_size_type size = 0, capacity = 0;
};

}  // namespace detail

template <class T>
using concurrent_pool_allocator_t = std::conditional_t<
    std::is_trivially_copyable_v<T>,
    yk::default_init_allocator<T>,
    std::allocator<T>
>;

template <class T, class BufT, concurrent_pool_flag Flags = concurrent_pool_flag::mpmc>
class concurrent_pool {
public:
  static constexpr concurrent_pool_flag flags = Flags;
  using value_type                            = T;
  using buf_type                              = BufT;
  using traits_type                           = detail::concurrent_pool_traits<T, BufT, Flags>;
  using condition_variable_type               = typename traits_type::condition_variable_type;
  using size_type                             = typename traits_type::size_type;

  // -------------------------------------------

  [[nodiscard]]
  size_type capacity() const
  {
    std::unique_lock lock{mtx_};
    return capacity_;
  }

  void set_capacity(size_type new_capacity)
  {
    if (new_capacity < 0) {
      throw std::length_error("new capacity must be non-negative");
    }
    std::unique_lock lock{mtx_};
    capacity_ = new_capacity;
  }

  void reserve(size_type new_capacity)
    requires traits_type::has_reserve
  {
    if (new_capacity < 0) {
      throw std::length_error("new capacity must be non-negative");
    }
    std::unique_lock lock{mtx_};
    capacity_ = new_capacity;
    buf_.reserve(static_cast<std::size_t>(capacity_));
  }

  // Note: this holds only the current state.
  // If you need a consistent value, close() the pool first.
  [[nodiscard]]
  size_type size() const
  {
    std::unique_lock lock{mtx_};
    return static_cast<size_type>(buf_.size());
  }

  // use `size() == 0` instead.
  //
  // This is deleted to prevent mistakes like below:
  //
  // if (cp.empty()) {
  //   do_something(cp.size()); // may be `0`
  // }
  bool empty() const = delete;

  // Atomically fetches both size() and capacity().
  // Intended for use in dynamic size adjustments.
  //
  // Note: this holds only the current state.
  // If you need a consistent value, close() the pool first.
  [[nodiscard]]
  detail::concurrent_pool_size_info size_info() const
  {
    std::unique_lock lock{mtx_};
    return {.size = static_cast<size_type>(buf_.size()), .capacity = capacity_};
  }

  // -------------------------------------------

  template <class U>
  [[nodiscard]]
  bool push_wait(U&& value)
  {
    std::unique_lock lock{mtx_};
    cv_not_full_.wait(lock, push_wait_cond());
    if (push_wait_cond_error()) {
      return false;
    }

    traits_type::push(buf_, std::forward<U>(value), cv_not_empty_);
    return true;
  }

#if __cpp_lib_jthread >= 201911L
  template <class U>
  bool push_wait(U&& value, std::stop_token stop_token)
    requires (!traits_type::has_stop_token_support)
  = delete;

  template <class U>
  [[nodiscard]]
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

    traits_type::push(buf_, std::forward<U>(value), cv_not_empty_);
    return true;
  }
#endif

  // -------------------------------------------

  [[nodiscard]]
  bool pop_wait(T& value)
  {
    std::unique_lock lock{mtx_};
    cv_not_empty_.wait(lock, pop_wait_cond());
    if (pop_wait_cond_error()) {
      return false;
    }

    if constexpr (traits_type::is_single_producer && traits_type::is_single_consumer) {
      traits_type::pop(buf_, value, cv_not_full_, capacity_);

    } else {
      traits_type::pop(buf_, value, cv_not_full_);
    }
    return true;
  }

#if __cpp_lib_jthread >= 201911L
  bool pop_wait(T& value, std::stop_token stop_token)
    requires (!traits_type::has_stop_token_support)
  = delete;

  [[nodiscard]]
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

    if constexpr (traits_type::is_single_producer && traits_type::is_single_consumer) {
      traits_type::pop(buf_, value, cv_not_full_, capacity_);

    } else {
      traits_type::pop(buf_, value, cv_not_full_);
    }
    return true;
  }
#endif

  // -------------------------------------------

  void close()
  {
    std::unique_lock lock{mtx_};
    closed_ = true;
    cv_not_full_.notify_all();
    cv_not_empty_.notify_all();
  }

  void open()
  {
    std::unique_lock lock{mtx_};
    closed_ = false;
    cv_not_full_.notify_all();
    cv_not_empty_.notify_all();
  }

  void clear()
  {
    std::unique_lock lock{mtx_};
    buf_.clear();
    cv_not_full_.notify_all();
    // cv_not_empty_.notify_all();
  }

private:
  [[nodiscard]]
  auto push_wait_cond() const
  {
    return [this] {
      return static_cast<size_type>(buf_.size()) < capacity_ || closed_;
    };
  }

  [[nodiscard]]
  bool push_wait_cond_error() const
  {
    return closed_;
  }

  [[nodiscard]]
  auto pop_wait_cond() const
  {
    return [this] {
      return !buf_.empty() || closed_;
    };
  }

  [[nodiscard]]
  bool pop_wait_cond_error() const
  {
    return closed_;
  }

  mutable std::mutex mtx_;
  buf_type buf_;
  size_type capacity_ = 1024;

  condition_variable_type cv_not_full_, cv_not_empty_;

  bool closed_ = false;
};

}  // namespace yk

#endif
