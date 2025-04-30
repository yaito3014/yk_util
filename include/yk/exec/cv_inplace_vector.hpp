#ifndef YK_EXEC_CV_INPLACE_VECTOR_HPP
#define YK_EXEC_CV_INPLACE_VECTOR_HPP

#include "yk/exec/cv_queue.hpp"

#include "yk/inplace_vector.hpp"

namespace yk::exec {

template <class T, std::size_t N, cv_queue_flag Flags>
using cv_inplace_vector = cv_queue<T, sg14::inplace_vector<T, N, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, std::size_t N, cv_queue_flag Flags = {}>
using spsc_cv_inplace_vector = spsc_cv_queue<T, sg14::inplace_vector<T, N, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, std::size_t N, cv_queue_flag Flags = {}>
using mpmc_cv_inplace_vector = mpmc_cv_queue<T, sg14::inplace_vector<T, N, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, std::size_t N, cv_queue_flag Flags = {}>
using spmc_cv_inplace_vector = spmc_cv_queue<T, sg14::inplace_vector<T, N, yk::exec::cv_queue_allocator_t<T>>, Flags>;

template <class T, std::size_t N, cv_queue_flag Flags = {}>
using mpsc_cv_inplace_vector = mpsc_cv_queue<T, sg14::inplace_vector<T, N, yk::exec::cv_queue_allocator_t<T>>, Flags>;

}  // yk::exec

#endif
