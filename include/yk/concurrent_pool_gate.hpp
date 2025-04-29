#ifndef YK_CONCURRENT_POOL_GATE_HPP
#define YK_CONCURRENT_POOL_GATE_HPP

#include "yk/concurrent_pool_types.hpp"
#include "yk/enum_bitops.hpp"
#include "yk/throwt.hpp"

#include <boost/assert.hpp>

#include <version>
#include <stdexcept>

#if __cpp_lib_jthread >= 201911L
#include <stop_token>
#endif

namespace yk {

namespace detail {

using concurrent_pool_gate_store_counter_type = long long;

enum struct concurrent_pool_gate_flags : unsigned {
  not_counted = 0b0,
  counted     = 0b1,

  need_info   = 0b10,
};

} // detail

template<>
struct ::yk::bitops_enabled<detail::concurrent_pool_gate_flags> : std::true_type {};


namespace detail {

template <class PoolT, concurrent_pool_gate_flags flags>
struct concurrent_pool_gate_store_base
{
  static constexpr bool is_counted = contains(flags, concurrent_pool_gate_flags::counted);
  static constexpr bool need_info  = contains(flags, concurrent_pool_gate_flags::need_info);

  using access_result_type = std::conditional_t<need_info, concurrent_pool_access_result, bool>;

  template <class Derived>
  concurrent_pool_gate_store_counter_type count(this const Derived& self) noexcept
    requires (!is_counted) = delete;

  template <class Derived>
  [[nodiscard]]
  concurrent_pool_gate_store_counter_type count(this const Derived& self) noexcept
    requires (is_counted)
  {
    return self.count_;
  }

  // -----------------------------------------

  [[nodiscard]]
  bool is_discarded(this const auto& self) noexcept requires (!is_counted)
  {
    bool const discarded = !self.pool_;
    BOOST_ASSERT(discarded || self.already_accessed_);
    return discarded;
  }

  [[nodiscard]]
  bool is_discarded(this const auto& self) noexcept requires (is_counted)
    = delete; // gate.discard() on a counted gate has no effect


  void discard(this auto& self) requires (!is_counted)
  {
    if (!self.pool_) {
      throwt<std::logic_error>("gate.discard() has been called multiple times");
    }
    self.pool_ = nullptr;
  }

  void discard(this const auto& self) requires (is_counted)
    = delete; // gate.discard() on a counted gate has no effect

  // -----------------------------------------

  [[nodiscard]]
  concurrent_pool_size_type last_pool_size(this const auto& self) noexcept requires (need_info)
  {
    return self.last_pool_size_;
  }

  concurrent_pool_size_type last_pool_size(this const auto& self) noexcept requires (!need_info)
    = delete;
};


template <class PoolT, concurrent_pool_gate_flags flags>
struct concurrent_pool_gate_store : concurrent_pool_gate_store_base<PoolT, flags>
{
  PoolT* pool_ = nullptr;

#ifndef NDEBUG
  bool already_accessed_ = false;
#endif
};

template <class PoolT>
struct concurrent_pool_gate_store<PoolT, concurrent_pool_gate_flags::counted>
  : concurrent_pool_gate_store_base<PoolT, concurrent_pool_gate_flags::counted>
{
  PoolT* pool_ = nullptr;
  concurrent_pool_gate_store_counter_type count_ = 0;
};

template <class PoolT>
struct concurrent_pool_gate_store<PoolT, concurrent_pool_gate_flags::need_info>
  : concurrent_pool_gate_store_base<PoolT, concurrent_pool_gate_flags::need_info>
{
  PoolT* pool_ = nullptr;
  concurrent_pool_size_type last_pool_size_ = 0;

#ifndef NDEBUG
  bool already_accessed_ = false;
#endif
};

template <class PoolT>
struct concurrent_pool_gate_store<PoolT, concurrent_pool_gate_flags::counted | concurrent_pool_gate_flags::need_info>
  : concurrent_pool_gate_store_base<PoolT, concurrent_pool_gate_flags::counted | concurrent_pool_gate_flags::need_info>
{
  PoolT* pool_ = nullptr;
  concurrent_pool_gate_store_counter_type count_ = 0;
  concurrent_pool_size_type last_pool_size_ = 0;
};

}  // namespace detail


template <class PoolT, detail::concurrent_pool_gate_flags flags = detail::concurrent_pool_gate_flags::not_counted>
struct producer_gate
  : detail::concurrent_pool_gate_store<PoolT, flags>
{
  using base_type = detail::concurrent_pool_gate_store<PoolT, flags>;
  using pool_type = PoolT;
  using value_type = typename PoolT::value_type;

  /*explicit*/ producer_gate(PoolT* pool) noexcept
    : base_type{.pool_ = pool}
  {
    BOOST_ASSERT(pool != nullptr);
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

    if constexpr (base_type::need_info) {
      const auto result = this->pool_->push_wait_info(std::move(stop_token), std::forward<Args>(args)...);
      this->last_pool_size_ = result.size;
      return result.ok;

    } else {
      return this->pool_->push_wait(std::move(stop_token), std::forward<Args>(args)...);
    }
  }
#endif

  template <class... Args>
  [[nodiscard]]
  bool push_wait(Args&&... args)
  {
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

    if constexpr (base_type::need_info) {
      const auto result = this->pool_->push_wait_info(std::forward<Args>(args)...);
      this->last_pool_size_ = result.size;
      return result.ok;

    } else {
      return this->pool_->push_wait(std::forward<Args>(args)...);
    }
  }
};

template <class PoolT>
using counted_producer_gate = producer_gate<PoolT, detail::concurrent_pool_gate_flags::counted>;

template <class PoolT>
using info_producer_gate = producer_gate<PoolT, detail::concurrent_pool_gate_flags::need_info>;

template <class PoolT>
using counted_info_producer_gate = producer_gate<PoolT, detail::concurrent_pool_gate_flags::counted | detail::concurrent_pool_gate_flags::need_info>;


template <class PoolT, detail::concurrent_pool_gate_flags flags = detail::concurrent_pool_gate_flags::not_counted>
struct consumer_gate
  : detail::concurrent_pool_gate_store<PoolT, flags>
{
  using base_type = detail::concurrent_pool_gate_store<PoolT, flags>;
  using pool_type = PoolT;
  using value_type = typename PoolT::value_type;

  /*explicit*/ consumer_gate(PoolT* pool) noexcept
      : base_type{.pool_ = pool}
  {
    BOOST_ASSERT(pool != nullptr);
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

    if constexpr (base_type::need_info) {
      const auto result = this->pool_->pop_wait_info(std::move(stop_token), value);
      this->last_pool_size_ = result.size;
      return result.ok;

    } else {
      return this->pool_->pop_wait(std::move(stop_token), value);
    }
  }
#endif

  [[nodiscard]]
  bool pop_wait(value_type& value)
  {
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

    if constexpr (base_type::need_info) {
      const auto result = this->pool_->pop_wait_info(value);
      this->last_pool_size_ = result.size;
      return result.ok;

    } else {
      return this->pool_->pop_wait(value);
    }
  }
};

template <class PoolT>
using counted_consumer_gate = consumer_gate<PoolT, detail::concurrent_pool_gate_flags::counted>;

template <class PoolT>
using info_consumer_gate = consumer_gate<PoolT, detail::concurrent_pool_gate_flags::need_info>;

template <class PoolT>
using counted_info_consumer_gate = consumer_gate<PoolT, detail::concurrent_pool_gate_flags::counted | detail::concurrent_pool_gate_flags::need_info>;

}  // namespace yk

#endif
