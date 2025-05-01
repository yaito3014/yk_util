#ifndef YK_EXEC_CV_QUEUE_HPP
#define YK_EXEC_CV_QUEUE_HPP

#include "yk/exec/cv_queue_types.hpp"
#include "yk/exec/queue_traits.hpp"

#include "yk/allocator/default_init_allocator.hpp"
#include "yk/util/to_underlying.hpp"

#include "yk/enum_bitops.hpp"
#include "yk/interrupt_exception.hpp"
#include "yk/throwt.hpp"

#include "yk/arch.hpp"

#include <version>

#if __cpp_lib_jthread >= 201911L
#include <stop_token>
#endif

#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <concepts>

#include <cstddef>

namespace yk::exec {

enum struct cv_queue_flag : unsigned {
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

} // yk::exec


namespace yk {

template <>
struct bitops_enabled<::yk::exec::cv_queue_flag> : std::true_type {};

} // yk


namespace yk::exec {

template <class T>
concept CVQueueValue =
  // unconditionally required for push/pop operations
  std::movable<T> || std::copyable<T>
;

struct [[nodiscard]] cv_queue_size_info
{
    detail::cv_queue_size_type size = 0, capacity = 0;
};

namespace detail {

template <CVQueueValue T, class BufT, cv_queue_flag Flags>
struct cv_queue_traits
{
  using value_type = T;
  using buf_type = BufT;
  using size_type = cv_queue_size_type;

  static constexpr cv_queue_flag flags   = Flags;
  static constexpr bool is_multi_producer       = static_cast<bool>(flags & cv_queue_flag::multi_producer);
  static constexpr bool is_single_producer      = !is_multi_producer;
  static constexpr bool is_multi_consumer       = static_cast<bool>(flags & cv_queue_flag::multi_consumer);
  static constexpr bool is_single_consumer      = !is_multi_consumer;

  static constexpr bool is_queue_based_push_pop = static_cast<bool>(flags & cv_queue_flag::queue_based_push_pop);

#if __cpp_lib_jthread >= 201911L
  static constexpr bool enable_stop_token_support = static_cast<bool>(flags & cv_queue_flag::stop_token_support);
#else
  static constexpr bool enable_stop_token_support = false;
  static_assert(!static_cast<bool>(flags & cv_queue_flag::stop_token_support), "stop_token_support cannot be enabled on this toolchain without std::stop_token");
#endif

  using condition_variable_type = std::conditional_t<
    enable_stop_token_support,
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

  static constexpr size_type default_capacity   = 1024;

  // ---------------------------------------

  template <class... Args>
  static void push(BufT& buf, condition_variable_type& cv_not_empty, Args&&... args)
    requires is_single_producer && is_single_consumer
  {
    const bool was_empty = buf.empty();
    do_push(buf, std::forward<Args>(args)...);

    if (was_empty) {
      cv_not_empty.notify_one();
    }
  }

  template <class... Args>
  static void push(BufT& buf, condition_variable_type& cv_not_empty, Args&&... args)
    requires (!(is_single_producer && is_single_consumer))
  {
    do_push(buf, std::forward<Args>(args)...);

    if constexpr (is_single_consumer) {
      cv_not_empty.notify_one();
    } else {
      cv_not_empty.notify_all();
    }
  }

  static void pop(BufT& buf, condition_variable_type& cv_not_full, size_type capacity, T& value)
    requires is_single_producer && is_single_consumer
  {
    const bool was_full = static_cast<size_type>(buf.size()) >= capacity;
    do_pop(buf, value);

    if (was_full) {
      cv_not_full.notify_one();
    }
  }

  static void pop(BufT& buf, condition_variable_type& cv_not_full, T& value)
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
  template <class... Args>
  static void do_push(BufT& buf, Args&&... args)
  {
    if constexpr (has_back_access) {
      buf.emplace_back(std::forward<Args>(args)...);

    } else {
      buf.emplace(std::forward<Args>(args)...);
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

}  // namespace detail

template <class T>
using cv_queue_allocator_t = std::conditional_t<
    std::is_trivially_copyable_v<T>,
    yk::default_init_allocator<T>,
    std::allocator<T>
>;

// Condition-Variable based concurrent Queue, aka cv_queue
template <class T, class BufT, cv_queue_flag Flags>
class cv_queue
{
public:
  static_assert(CVQueueValue<T>);

  static constexpr cv_queue_flag flags = Flags;
  using value_type                     = T;
  using buf_type                       = BufT;
  using traits_type                    = detail::cv_queue_traits<T, BufT, Flags>;
  using condition_variable_type        = typename traits_type::condition_variable_type;
  using size_type                      = typename traits_type::size_type;

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
      throwt<std::length_error>("new capacity must be non-negative");
    }
    std::unique_lock lock{mtx_};
    if (static_cast<typename BufT::size_type>(new_capacity) > buf_.max_size()) {
      throwt<std::length_error>("new capacity ({}) exceeds underlying buffer's max_size ({})", new_capacity, buf_.max_size());
    }
    capacity_ = new_capacity;
  }

  void reserve_capacity() requires traits_type::has_reserve
  {
    std::unique_lock lock{mtx_};
    buf_.reserve(static_cast<typename decltype(buf_)::size_type>(capacity_));
  }

  void set_capacity_and_reserve(size_type new_capacity) requires traits_type::has_reserve
  {
    if (new_capacity < 0) {
      throwt<std::length_error>("new capacity must be non-negative");
    }
    std::unique_lock lock{mtx_};
    if (static_cast<typename BufT::size_type>(new_capacity) > buf_.max_size()) {
      throwt<std::length_error>("new capacity ({}) exceeds underlying buffer's max_size ({})", new_capacity, buf_.max_size());
    }
    capacity_ = new_capacity;

    buf_.reserve(static_cast<typename decltype(buf_)::size_type>(capacity_));
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
  cv_queue_size_info size_info() const
  {
    std::unique_lock lock{mtx_};
    return {.size = static_cast<size_type>(buf_.size()), .capacity = capacity_};
  }

  // -------------------------------------------

  template <class... Args>
  [[nodiscard]]
  bool push_wait(Args&&... args)
  {
    std::unique_lock lock{mtx_};
    cv_not_full_.wait(lock, push_wait_cond());
    if (push_wait_cond_error()) {
      return false;
    }

    traits_type::push(buf_, cv_not_empty_, std::forward<Args>(args)...);
    return true;
  }

#if __cpp_lib_jthread >= 201911L
  template <class... Args>
  bool push_wait(std::stop_token stop_token, Args&&... args)
    requires (!traits_type::enable_stop_token_support)
  = delete;

  template <class... Args>
  [[nodiscard]]
  bool push_wait(std::stop_token stop_token, Args&&... args)
    requires traits_type::enable_stop_token_support
  {
    std::unique_lock lock{mtx_};
    cv_not_full_.wait(lock, stop_token, push_wait_cond());
    if (stop_token.stop_requested()) {
      throwt<interrupt_exception>();
    }
    if (push_wait_cond_error()) {
      return false;
    }

    traits_type::push(buf_, cv_not_empty_, std::forward<Args>(args)...);
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
      traits_type::pop(buf_, cv_not_full_, capacity_, value);

    } else {
      traits_type::pop(buf_, cv_not_full_, value);
    }
    return true;
  }

#if __cpp_lib_jthread >= 201911L
  bool pop_wait(std::stop_token stop_token, T& value)
    requires (!traits_type::enable_stop_token_support)
  = delete;

  [[nodiscard]]
  bool pop_wait(std::stop_token stop_token, T& value)
    requires traits_type::enable_stop_token_support
  {
    std::unique_lock lock{mtx_};
    cv_not_empty_.wait(lock, stop_token, pop_wait_cond());
    if (stop_token.stop_requested()) {
      throwt<interrupt_exception>();
    }
    if (pop_wait_cond_error()) {
      return false;
    }

    if constexpr (traits_type::is_single_producer && traits_type::is_single_consumer) {
      traits_type::pop(buf_, cv_not_full_, capacity_, value);

    } else {
      traits_type::pop(buf_, cv_not_full_, value);
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

YK_FORCEALIGN_BEGIN
  alignas(yk::hardware_destructive_interference_size) mutable std::mutex mtx_;
  alignas(yk::hardware_destructive_interference_size) condition_variable_type cv_not_full_, cv_not_empty_;
YK_FORCEALIGN_END

  buf_type buf_;
  size_type capacity_ = traits_type::default_capacity;
  bool closed_ = false;
};


template <class T, class BufT, cv_queue_flag Flags = {}>
using spsc_cv_queue = cv_queue<
  T, BufT,
  (Flags & ~cv_queue_flag::producer_consumer_mask) | cv_queue_flag::spsc
>;

template <class T, class BufT, cv_queue_flag Flags = {}>
using mpmc_cv_queue = cv_queue<
  T, BufT,
  (Flags & ~cv_queue_flag::producer_consumer_mask) | cv_queue_flag::mpmc
>;

template <class T, class BufT, cv_queue_flag Flags = {}>
using spmc_cv_queue = cv_queue<
  T, BufT,
  (Flags & ~cv_queue_flag::producer_consumer_mask) | cv_queue_flag::spmc
>;

template <class T, class BufT, cv_queue_flag Flags = {}>
using mpsc_cv_queue = cv_queue<
  T, BufT,
  (Flags & ~cv_queue_flag::producer_consumer_mask) | cv_queue_flag::mpsc
>;


template <class T, class BufT, cv_queue_flag Flags>
struct queue_traits<cv_queue<T, BufT, Flags>>
{
  static constexpr bool need_stop_token_for_cancel = false;
};

template <class T, class BufT, cv_queue_flag Flags>
struct queue_access<cv_queue<T, BufT, Flags>>
{
  using queue_type = cv_queue<T, BufT, Flags>;

  template <class... Args>
  [[nodiscard]]
  static bool cancelable_push(queue_type& queue, Args&&... args)
  {
    return queue.push_wait(std::forward<Args>(args)...);
  }

  [[nodiscard]]
  static bool cancelable_pop(queue_type& queue, T& value)
  {
    return queue.pop_wait(value);
  }
};

}  // yk::exec

#endif
