#ifndef YK_EXEC_WORKER_TYPES_HPP
#define YK_EXEC_WORKER_TYPES_HPP

#include "yk/exec/thread_index.hpp"

#include <ranges>
#include <concepts>


namespace yk::exec {

enum struct worker_mode_t : bool
{
  producer,
  consumer,
};

enum struct producer_kind : bool
{
  single_push,
  multi_push,
};

enum struct consumer_kind : bool
{
  single_pop,
  multi_pop,
};

template <class R>
concept ProducerInputRange = std::ranges::forward_range<R>;

template <class F, class ProducerInputRangeT, class GateT>
concept Producer =
  ProducerInputRange<ProducerInputRangeT> &&
  std::invocable<F, thread_index_t, std::ranges::range_value_t<ProducerInputRangeT>, GateT&>
;

template <class F, class GateT>
concept Consumer = std::invocable<F, thread_index_t, GateT&>;

} // yk::exec

#endif
