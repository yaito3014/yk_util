#ifndef YK_EXEC_CONCURRENT_GATE_HPP
#define YK_EXEC_CONCURRENT_GATE_HPP

#include "yk/enum_bitops.hpp"
#include "yk/throwt.hpp"

#include <boost/assert.hpp>

#if YK_EXEC_DEBUG
#include <chrono>
#endif

#include <version>
#include <stdexcept>

#if __cpp_lib_jthread >= 201911L
#include <stop_token>
#endif

namespace yk::exec::detail {

using concurrent_gate_store_counter_type = long long;

enum struct concurrent_gate_flags : unsigned {
  not_counted = 0b0,
  counted     = 0b1,
};

} // yk::exec::detail


namespace yk {

template<> struct bitops_enabled<exec::detail::concurrent_gate_flags> : std::true_type {};

} // yk


namespace yk::exec {
namespace detail {

template <class QueueT, concurrent_gate_flags flags>
struct concurrent_gate_store_base
{
  static constexpr bool is_counted = contains(flags, concurrent_gate_flags::counted);

  static constexpr bool is_lock_free = requires(QueueT& queue) {
    typename QueueT::value_type;
    { queue.is_lock_free() } -> std::convertible_to<bool>;
    { queue.push(std::declval<typename QueueT::value_type&>()) } -> std::convertible_to<bool>;
  };

  template <class Derived>
  concurrent_gate_store_counter_type count(this const Derived& self) noexcept
    requires (!is_counted) = delete;

  template <class Derived>
  [[nodiscard]]
  concurrent_gate_store_counter_type count(this const Derived& self) noexcept
    requires (is_counted)
  {
    return self.count_;
  }

  // -----------------------------------------

  [[nodiscard]]
  bool is_discarded(this const auto& self) noexcept requires (!is_counted)
  {
    bool const discarded = !self.queue_;
    BOOST_ASSERT(discarded || self.already_accessed_);
    return discarded;
  }

  [[nodiscard]]
  bool is_discarded(this const auto& self) noexcept requires (is_counted)
    = delete; // gate.discard() on a counted gate has no effect


  void discard(this auto& self) requires (!is_counted)
  {
    if (!self.queue_) {
      throwt<std::logic_error>("gate.discard() has been called multiple times");
    }
    self.queue_ = nullptr;
  }

  void discard(this const auto& self) requires (is_counted)
    = delete; // gate.discard() on a counted gate has no effect

  // -----------------------------------------

#if YK_EXEC_DEBUG
  [[nodiscard]] std::chrono::nanoseconds elapsed_time() const noexcept { return elapsed_time_; }
#endif

protected:
#if YK_EXEC_DEBUG
  std::chrono::nanoseconds elapsed_time_{};

  void add_time(std::chrono::nanoseconds elapsed_time)
  {
      elapsed_time_ += elapsed_time;
  }

  friend struct auto_timer;
  struct auto_timer
  {
    concurrent_gate_store_base* base = nullptr;

    using clock_type = std::chrono::steady_clock;
    clock_type::time_point start_time;

    auto_timer(concurrent_gate_store_base* base) : base(base), start_time(clock_type::now()) {}
    ~auto_timer() { base->add_time(clock_type::now() - start_time); }
  };
#endif
};


template <class QueueT, concurrent_gate_flags flags>
struct concurrent_gate_store : concurrent_gate_store_base<QueueT, flags>
{
  QueueT* queue_ = nullptr;

#ifndef NDEBUG
  bool already_accessed_ = false;
#endif
};

template <class QueueT>
struct concurrent_gate_store<QueueT, concurrent_gate_flags::counted>
  : concurrent_gate_store_base<QueueT, concurrent_gate_flags::counted>
{
  QueueT* queue_ = nullptr;
  concurrent_gate_store_counter_type count_ = 0;
};

}  // namespace detail


template <class QueueT, detail::concurrent_gate_flags flags = detail::concurrent_gate_flags::not_counted>
struct producer_gate
  : detail::concurrent_gate_store<QueueT, flags>
{
  using base_type = detail::concurrent_gate_store<QueueT, flags>;
  using queue_type = QueueT;
  using value_type = typename QueueT::value_type;

  /*explicit*/ producer_gate(QueueT* queue) noexcept
    : base_type{.queue_ = queue}
  {
    BOOST_ASSERT(queue != nullptr);
  }

  producer_gate() = delete;
  producer_gate(const producer_gate&) noexcept = delete;
  producer_gate(producer_gate&&) noexcept = delete;

  producer_gate& operator=(const producer_gate&) noexcept = delete;
  producer_gate& operator=(producer_gate&&) noexcept = delete;

#if __cpp_lib_jthread >= 201911L
  template <class... Args>
  [[nodiscard]]
  bool push_wait(std::stop_token stop_token, Args&&... args)
  {
#if YK_EXEC_DEBUG
    typename base_type::auto_timer timer{this};
#endif

    if constexpr (base_type::is_counted) {
      ++this->count_;

#ifndef NDEBUG
    } else {
      if (this->already_accessed_) {
        throwt<std::logic_error>(
          "non-counted gate has been accessed multiple times, "
          "this will lead to untrackable progress on schedulers; "
          "consider using multi-push producer"
        );
      }
      this->already_accessed_ = true;
#endif
    }

    return this->queue_->push_wait(std::move(stop_token), std::forward<Args>(args)...);
  }
#endif

  template <class... Args>
  [[nodiscard]]
  bool push_wait(Args&&... args)
  {
#if YK_EXEC_DEBUG
    typename base_type::auto_timer timer{this};
#endif

    if constexpr (base_type::is_counted) {
      ++this->count_;

#ifndef NDEBUG
    } else {
      if (this->already_accessed_) {
        throwt<std::logic_error>(
          "non-counted gate has been accessed multiple times, "
          "this will lead to untrackable progress on schedulers; "
          "consider using multi-push producer"
        );
      }
      this->already_accessed_ = true;
#endif
    }

    if constexpr (base_type::is_lock_free) {
      while (!this->queue_->bounded_push(std::forward<Args>(args)...)) {
        //std::this_thread::yield();
      }
      return true;

    } else {
      return this->queue_->push_wait(std::forward<Args>(args)...);
    }
  }
};

template <class QueueT>
using counted_producer_gate = producer_gate<QueueT, detail::concurrent_gate_flags::counted>;


template <class QueueT, detail::concurrent_gate_flags flags = detail::concurrent_gate_flags::not_counted>
struct consumer_gate
  : detail::concurrent_gate_store<QueueT, flags>
{
  using base_type = detail::concurrent_gate_store<QueueT, flags>;
  using queue_type = QueueT;
  using value_type = typename QueueT::value_type;

  /*explicit*/ consumer_gate(QueueT* queue) noexcept
    : base_type{.queue_ = queue}
  {
    BOOST_ASSERT(queue != nullptr);
  }

  consumer_gate() = delete;
  consumer_gate(const consumer_gate&) noexcept = delete;
  consumer_gate(consumer_gate&&) noexcept = delete;

  consumer_gate& operator=(const consumer_gate&) noexcept = delete;
  consumer_gate& operator=(consumer_gate&&) noexcept = delete;

#if __cpp_lib_jthread >= 201911L
  [[nodiscard]]
  bool pop_wait(std::stop_token stop_token, value_type& value)
  {
#if YK_EXEC_DEBUG
    typename base_type::auto_timer timer{this};
#endif

    if constexpr (base_type::is_counted) {
      ++this->count_;

#ifndef NDEBUG
    } else {
      if (this->already_accessed_) {
        throwt<std::logic_error>(
          "non-counted gate has been accessed multiple times, "
          "this will lead to untrackable progress on schedulers; "
          "consider using multi-pop consumer"
        );
      }
      this->already_accessed_ = true;
#endif
    }

    return this->queue_->pop_wait(std::move(stop_token), value);
  }
#endif

  [[nodiscard]]
  bool pop_wait(value_type& value)
  {
#if YK_EXEC_DEBUG
    typename base_type::auto_timer timer{this};
#endif

    if constexpr (base_type::is_counted) {
      ++this->count_;

#ifndef NDEBUG
    } else {
      if (this->already_accessed_) {
        throwt<std::logic_error>(
          "non-counted gate has been accessed multiple times, "
          "this will lead to untrackable progress on schedulers; "
          "consider using multi-pop consumer"
        );
      }
      this->already_accessed_ = true;
#endif
    }

    if constexpr (base_type::is_lock_free) {
      while (!this->queue_->pop(value)) {
        //std::this_thread::yield();
      }
      return true;

    } else {
      return this->queue_->pop_wait(value);
    }
  }
};

template <class QueueT>
using counted_consumer_gate = consumer_gate<QueueT, detail::concurrent_gate_flags::counted>;

}  // yk::exec

#endif
