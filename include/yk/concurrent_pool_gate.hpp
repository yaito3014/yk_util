#ifndef YK_CONCURRENT_POOL_GATE_HPP
#define YK_CONCURRENT_POOL_GATE_HPP

#if __cpp_lib_jthread >= 201911L
#include <stop_token>
#endif

namespace yk {

namespace detail {

using concurrent_pool_gate_store_counter_type = long long;

enum struct concurrent_pool_gate_kind : bool {
  not_counted,
  counted,
};

template <class PoolT, concurrent_pool_gate_kind kind>
struct concurrent_pool_gate_store;

template <class PoolT>
struct concurrent_pool_gate_store<PoolT, concurrent_pool_gate_kind::not_counted> {
  PoolT* pool_ = nullptr;
};

template <class PoolT>
struct concurrent_pool_gate_store<PoolT, concurrent_pool_gate_kind::counted> {
  PoolT* pool_ = nullptr;
  concurrent_pool_gate_store_counter_type count_ = 0;
};

}  // namespace detail

template <class PoolT, detail::concurrent_pool_gate_kind kind = detail::concurrent_pool_gate_kind::not_counted>
struct producer_gate : protected detail::concurrent_pool_gate_store<PoolT, kind> {
  /*explicit*/ producer_gate(PoolT* pool) noexcept  //
      : detail::concurrent_pool_gate_store<PoolT, kind>{pool, 0ll} {}

  producer_gate() = delete;
  producer_gate(const producer_gate&) noexcept = delete;
  producer_gate(producer_gate&&) noexcept = delete;

  producer_gate& operator=(const producer_gate&) noexcept = delete;
  producer_gate& operator=(producer_gate&&) noexcept = delete;

#if __cpp_lib_jthread >= 201911L
  template <class T>
  [[nodiscard]]
  bool push_wait(T&& value, std::stop_token stop_token) {
    if constexpr (kind == detail::concurrent_pool_gate_kind::counted) {
      ++this->count_;
    }
    return this->pool_->push_wait(std::forward<T>(value), std::move(stop_token));
  }
#endif

  template <class T>
  [[nodiscard]]
  bool push_wait(T&& value) {
    if constexpr (kind == detail::concurrent_pool_gate_kind::counted) {
      ++this->count_;
    }
    return this->pool_->push_wait(std::forward<T>(value));
  }

  [[nodiscard]]
  detail::concurrent_pool_gate_store_counter_type count() const noexcept
    requires (kind == detail::concurrent_pool_gate_kind::counted)
  {
    return this->count_;
  }
};

template <class PoolT>
using counted_producer_gate = producer_gate<PoolT, detail::concurrent_pool_gate_kind::counted>;

template <class PoolT, detail::concurrent_pool_gate_kind kind = detail::concurrent_pool_gate_kind::not_counted>
struct consumer_gate : protected detail::concurrent_pool_gate_store<PoolT, kind> {
  /*explicit*/ consumer_gate(PoolT* pool) noexcept  //
      : detail::concurrent_pool_gate_store<PoolT, kind>{pool, 0ll} {}

  consumer_gate() = delete;
  consumer_gate(const consumer_gate&) noexcept = delete;
  consumer_gate(consumer_gate&&) noexcept = delete;

  consumer_gate& operator=(const consumer_gate&) noexcept = delete;
  consumer_gate& operator=(consumer_gate&&) noexcept = delete;

#if __cpp_lib_jthread >= 201911L
  template <class T>
  [[nodiscard]]
  bool pop_wait(T& value, std::stop_token stop_token) {
    if constexpr (kind == detail::concurrent_pool_gate_kind::counted) {
      ++this->count_;
    }
    return this->pool_->pop_wait(value, std::move(stop_token));
  }
#endif

  template <class T>
  [[nodiscard]]
  bool pop_wait(T& value) {
    if constexpr (kind == detail::concurrent_pool_gate_kind::counted) {
      ++this->count_;
    }
    return this->pool_->pop_wait(value);
  }

  [[nodiscard]]
  detail::concurrent_pool_gate_store_counter_type count() const noexcept
    requires (kind == detail::concurrent_pool_gate_kind::counted)
  {
    return this->count_;
  }
};

template <class PoolT>
using counted_consumer_gate = consumer_gate<PoolT, detail::concurrent_pool_gate_kind::counted>;

}  // namespace yk

#endif
