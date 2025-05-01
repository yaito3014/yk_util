#ifndef YK_EXEC_SCHEDULER_TRAITS_HPP
#define YK_EXEC_SCHEDULER_TRAITS_HPP

#include "yk/exec/debug.hpp"// for ODR violation safety
#include "yk/exec/worker_types.hpp"
#include "yk/exec/queue_gate.hpp"

#include <ranges>
#include <concepts>
#include <type_traits>

namespace yk::exec {

template <
  producer_kind ProducerKind,
  consumer_kind ConsumerKind,
  ProducerInputRange ProducerInputRangeT,
  class QueueT
>
struct scheduler_traits
{
  static constexpr bool is_multi_push = ProducerKind == producer_kind::multi_push;
  static constexpr bool is_multi_pop = ConsumerKind == consumer_kind::multi_pop;

  using queue_type = QueueT;
  using value_type = typename QueueT::value_type;

  using producer_input_value_type = std::ranges::range_value_t<ProducerInputRangeT>;
  using producer_input_iterator = std::ranges::iterator_t<ProducerInputRangeT>;

  using producer_gate_type = std::conditional_t<
    is_multi_push,
    counted_producer_gate<QueueT>,
    producer_gate<QueueT>
  >;

  using consumer_gate_type = std::conditional_t<
    is_multi_pop,
    counted_consumer_gate<QueueT>,
    consumer_gate<QueueT>
  >;
};

} // yk::exec

#endif
