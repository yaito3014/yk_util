#ifndef YK_CONCURRENT_DEQUE_HPP
#define YK_CONCURRENT_DEQUE_HPP

#include "yk/concurrent_pool.hpp"

#include <deque>

namespace yk {

template <class T, concurrent_pool_flag Flags>
using concurrent_deque = concurrent_pool<T, std::deque<T, yk::concurrent_pool_allocator_t<T>>, Flags>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_spsc_deque = concurrent_spsc_pool<T, std::deque<T, yk::concurrent_pool_allocator_t<T>>, Flags>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_mpmc_deque = concurrent_mpmc_pool<T, std::deque<T, yk::concurrent_pool_allocator_t<T>>, Flags>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_spmc_deque = concurrent_spmc_pool<T, std::deque<T, yk::concurrent_pool_allocator_t<T>>, Flags>;

template <class T, concurrent_pool_flag Flags = {}>
using concurrent_mpsc_deque = concurrent_mpsc_pool<T, std::deque<T, yk::concurrent_pool_allocator_t<T>>, Flags>;

}  // namespace yk

#endif
