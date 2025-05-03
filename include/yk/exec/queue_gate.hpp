#ifndef YK_EXEC_QUEUE_GATE_HPP
#define YK_EXEC_QUEUE_GATE_HPP

#include "yk/exec/debug.hpp"
#include "yk/exec/worker_types.hpp"
#include "yk/exec/queue_traits.hpp"

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

using queue_gate_store_counter_type = long long;

enum struct queue_gate_flag : unsigned {
  not_counted = 0b0,
  counted     = 0b1,
};

} // yk::exec::detail


namespace yk {

template<> struct bitops_enabled<exec::detail::queue_gate_flag> : std::true_type {};

} // yk


namespace yk::exec {
namespace detail {

template <class QueueT, bool need_stop_token>
struct queue_gate_store_base_impl
{
  /*explicit*/ queue_gate_store_base_impl(QueueT* queue) noexcept
    : queue_(queue)
  {
    BOOST_ASSERT(queue != nullptr);
  }

protected:
  QueueT* queue_;
};

template <class QueueT>
struct queue_gate_store_base_impl<QueueT, true>
{
  /*explicit*/ queue_gate_store_base_impl(QueueT* queue, std::stop_token stop_token) noexcept
    : queue_(queue)
    , stop_token_(std::move(stop_token))
  {
    BOOST_ASSERT(queue != nullptr);
  }

protected:
  QueueT* queue_;
  std::stop_token stop_token_;
};

template <class QueueT, queue_gate_flag flags>
struct queue_gate_store_base
  : queue_gate_store_base_impl<QueueT, queue_traits<QueueT>::need_stop_token_for_cancel>
{
  static constexpr bool is_counted = contains(flags, queue_gate_flag::counted);

  // -----------------------------------------

  using queue_gate_store_base::queue_gate_store_base_impl::queue_gate_store_base_impl;

  template <class Derived>
  queue_gate_store_counter_type count(this const Derived& self) noexcept
    requires (!is_counted) = delete;

  template <class Derived>
  [[nodiscard]]
  queue_gate_store_counter_type count(this const Derived& self) noexcept
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
public:
  [[nodiscard]] std::chrono::nanoseconds elapsed_time() const noexcept { return elapsed_time_; }

protected:
  std::chrono::nanoseconds elapsed_time_{};

  void add_time(std::chrono::nanoseconds elapsed_time)
  {
      elapsed_time_ += elapsed_time;
  }

  friend struct auto_timer;
  struct auto_timer
  {
    queue_gate_store_base* base = nullptr;

    using clock_type = std::chrono::steady_clock;
    clock_type::time_point start_time;

    auto_timer(queue_gate_store_base* base) : base(base), start_time(clock_type::now()) {}
    ~auto_timer() { base->add_time(clock_type::now() - start_time); }
  };
#endif
};


template <class QueueT, queue_gate_flag flags>
struct queue_gate_store : queue_gate_store_base<QueueT, flags>
{
  using queue_gate_store::queue_gate_store_base::queue_gate_store_base;

#ifndef NDEBUG
  bool already_accessed_ = false;
#endif
};

template <class QueueT>
struct queue_gate_store<QueueT, queue_gate_flag::counted> : queue_gate_store_base<QueueT, queue_gate_flag::counted>
{
  using queue_gate_store::queue_gate_store_base::queue_gate_store_base;

  queue_gate_store_counter_type count_ = 0;
};

}  // namespace detail


template <class QueueT, worker_mode_t WorkerMode, detail::queue_gate_flag flags = detail::queue_gate_flag::not_counted>
struct queue_gate : detail::queue_gate_store<QueueT, flags>
{
private:
  using base_type = detail::queue_gate_store<QueueT, flags>;

public:
  using queue_type = QueueT;
  using traits_type = queue_traits<QueueT>;
  using value_type = typename traits_type::value_type;

  using base_type::base_type;

  queue_gate() = delete;
  queue_gate(const queue_gate&) noexcept = delete;
  queue_gate(queue_gate&&) noexcept = delete;

  queue_gate& operator=(const queue_gate&) noexcept = delete;
  queue_gate& operator=(queue_gate&&) noexcept = delete;


  template <class... Args>
  [[nodiscard]]
  bool push_wait(Args&&... args) requires (WorkerMode == worker_mode_t::producer)
  {
#if YK_EXEC_DEBUG
    typename base_type::auto_timer timer{this};
#endif

    this->mark_access();

    if constexpr (traits_type::need_stop_token_for_cancel) {
      return traits_type::cancelable_bounded_push(this->stop_token_, *this->queue_, std::forward<Args>(args)...);

    } else {
      return traits_type::cancelable_bounded_push(*this->queue_, std::forward<Args>(args)...);
    }
  }

  template <class... Args>
  [[nodiscard]]
  bool pop_wait(Args&&... args) requires (WorkerMode == worker_mode_t::consumer)
  {
#if YK_EXEC_DEBUG
    typename base_type::auto_timer timer{this};
#endif

    this->mark_access();

    if constexpr (traits_type::need_stop_token_for_cancel) {
      return traits_type::cancelable_pop(this->stop_token_, *this->queue_, std::forward<Args>(args)...);

    } else {
      return traits_type::cancelable_pop(*this->queue_, std::forward<Args>(args)...);
    }
  }

private:
  void mark_access()
  {
    if constexpr (base_type::is_counted) {
      ++this->count_;

#ifndef NDEBUG
    } else {
      if (this->already_accessed_) {
        throwt<std::logic_error>(
          "non-counted gate has been accessed multiple times, "
          "this will lead to untrackable progress on schedulers; "
          "consider using multi-push producer / multi-pop consumer"
        );
      }
      this->already_accessed_ = true;
#endif
    }
  }
};

template <class QueueT>
using producer_gate = queue_gate<QueueT, worker_mode_t::producer, detail::queue_gate_flag::not_counted>;

template <class QueueT>
using counted_producer_gate = queue_gate<QueueT, worker_mode_t::producer, detail::queue_gate_flag::counted>;

template <class QueueT>
using consumer_gate = queue_gate<QueueT, worker_mode_t::consumer, detail::queue_gate_flag::not_counted>;

template <class QueueT>
using counted_consumer_gate = queue_gate<QueueT, worker_mode_t::consumer, detail::queue_gate_flag::counted>;

}  // yk::exec

#endif
