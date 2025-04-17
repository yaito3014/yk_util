#ifndef YK_CONCURRENT_VECTOR_HPP
#define YK_CONCURRENT_VECTOR_HPP

#include "yk/allocator/default_init_allocator.hpp"
#include "yk/concurrent_pool.hpp"

#include <vector>

namespace yk {

// clang-format off

template <class T, concurrent_pool_flag Flags = concurrent_pool_flag::mpmc>
using concurrent_vector = concurrent_pool<
  T,
  std::vector<T, yk::concurrent_pool_allocator_t<T>>,
  Flags
>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_spsc_vector = concurrent_vector<
  T,
  (Flags & ~concurrent_pool_flag::producer_consumer_mask) | concurrent_pool_flag::spsc
>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_mpmc_vector = concurrent_vector<
  T,
  (Flags & ~concurrent_pool_flag::producer_consumer_mask) | concurrent_pool_flag::mpmc
>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_spmc_vector = concurrent_vector<
  T,
  (Flags & ~concurrent_pool_flag::producer_consumer_mask) | concurrent_pool_flag::spmc
>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_mpsc_vector = concurrent_vector<
  T,
  (Flags & ~concurrent_pool_flag::producer_consumer_mask) | concurrent_pool_flag::mpsc
>;

// clang-format on

}  // namespace yk

#endif
