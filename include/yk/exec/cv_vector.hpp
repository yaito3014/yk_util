#ifndef YK_EXEC_CV_VECTOR_HPP
#define YK_EXEC_CV_VECTOR_HPP

#include "yk/exec/cv_queue.hpp"

#include <vector>

namespace yk::exec {

template <class T, cv_queue_flag Flags>
using cv_vector = cv_queue<T, std::vector<T, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, cv_queue_flag Flags = {}>
using spsc_cv_vector = spsc_cv_queue<T, std::vector<T, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, cv_queue_flag Flags = {}>
using mpmc_cv_vector = mpmc_cv_queue<T, std::vector<T, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, cv_queue_flag Flags = {}>
using spmc_cv_vector = spmc_cv_queue<T, std::vector<T, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, cv_queue_flag Flags = {}>
using mpsc_cv_vector = mpsc_cv_queue<T, std::vector<T, yk::exec::cv_queue_allocator_t<T>>, Flags>;

}  // yk::exec

#endif
