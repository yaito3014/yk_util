#ifndef YK_EXEC_CV_DEQUE_HPP
#define YK_EXEC_CV_DEQUE_HPP

#include "yk/exec/cv_queue.hpp"

#include <deque>

namespace yk::exec {

template <class T, cv_queue_flag Flags>
using cv_deque = cv_queue<T, std::deque<T, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, cv_queue_flag Flags = {}>
using spsc_cv_deque = spsc_cv_queue<T, std::deque<T, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, cv_queue_flag Flags = {}>
using mpmc_cv_deque = mpmc_cv_queue<T, std::deque<T, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, cv_queue_flag Flags = {}>
using spmc_cv_deque = spmc_cv_queue<T, std::deque<T, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, cv_queue_flag Flags = {}>
using mpsc_cv_deque = mpsc_cv_queue<T, std::deque<T, yk::exec::cv_queue_allocator_t<T>>, Flags>;

}  // yk::exec

#endif
