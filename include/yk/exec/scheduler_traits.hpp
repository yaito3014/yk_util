#ifndef YK_EXEC_SCHEDULER_TRAITS_HPP
#define YK_EXEC_SCHEDULER_TRAITS_HPP

#include "yk/exec/debug.hpp"
#include "yk/concurrent_vector.hpp"
#include "yk/concurrent_pool_gate.hpp"

#include <ranges>
#include <concepts>

namespace yk::exec {

enum struct worker_mode_t : bool {
  producer,
  consumer,
};

template <class R>
concept ProducerInputRange = std::ranges::forward_range<R>;

template <class F, class T, class ProducerInputRangeT, class GateT>
concept Producer =
  ProducerInputRange<ProducerInputRangeT> &&
  std::invocable<F, thread_id_t, std::ranges::range_value_t<ProducerInputRangeT>, GateT&>
;

template <class F, class T>
concept Consumer = std::invocable<F, thread_id_t, T>;

template <ProducerInputRange ProducerInputRangeT, class T>
struct scheduler_traits {
  using value_type = T;

  using producer_input_value_type = std::ranges::range_value_t<ProducerInputRangeT>;
  using producer_input_iterator = std::ranges::iterator_t<ProducerInputRangeT>;

  using queue_type = yk::concurrent_mpmc_vector<T>;
  using producer_gate_type = yk::counted_producer_gate<queue_type>;
  using consumer_gate_type = yk::consumer_gate<queue_type>;
};

} // yk::exec

#endif
