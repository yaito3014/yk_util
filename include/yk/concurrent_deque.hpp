#ifndef YK_CONCURRENT_DEQUE_HPP
#define YK_CONCURRENT_DEQUE_HPP

#include "yk/concurrent_pool.hpp"

#include <deque>

namespace yk {

template <class T, concurrent_pool_flag Flags = concurrent_pool_flag::mpmc>
using concurrent_deque = concurrent_pool<
  T,
  std::deque<T, yk::concurrent_pool_allocator_t<T>>,
  Flags
>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_spsc_deque = concurrent_deque<
  T,
  (Flags & ~concurrent_pool_flag::producer_consumer_mask) | concurrent_pool_flag::spsc
>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_mpmc_deque = concurrent_deque<
  T,
  (Flags & ~concurrent_pool_flag::producer_consumer_mask) | concurrent_pool_flag::mpmc
>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_spmc_deque = concurrent_deque<
  T,
  (Flags & ~concurrent_pool_flag::producer_consumer_mask) | concurrent_pool_flag::spmc
>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_mpsc_deque = concurrent_deque<
  T,
  (Flags & ~concurrent_pool_flag::producer_consumer_mask) | concurrent_pool_flag::mpsc
>;

}  // namespace yk

#endif
