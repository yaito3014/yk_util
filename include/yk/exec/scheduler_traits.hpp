#ifndef YK_EXEC_SCHEDULER_TRAITS_HPP
#define YK_EXEC_SCHEDULER_TRAITS_HPP

#include "yk/exec/debug.hpp"
#include "yk/exec/thread_id.hpp" // intentionally included
#include "yk/concurrent_vector.hpp"
#include "yk/concurrent_pool_gate.hpp"

#include <ranges>
#include <concepts>
#include <type_traits>

namespace yk::exec {

namespace detail {
enum struct worker_mode_t : bool
{
  producer,
  consumer,
};
} // detail

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

// convenient access via `yk::exec::...`

inline constexpr producer_kind single_push_producer = producer_kind::single_push;
inline constexpr producer_kind multi_push_producer = producer_kind::multi_push;
inline constexpr consumer_kind single_pop_consumer = consumer_kind::single_pop;
inline constexpr consumer_kind multi_pop_consumer = consumer_kind::multi_pop;

template <class R>
concept ProducerInputRange = std::ranges::forward_range<R>;

template <class F, class T, class ProducerInputRangeT, class GateT>
concept Producer =
  ProducerInputRange<ProducerInputRangeT> &&
  std::invocable<F, thread_id_t, std::ranges::range_value_t<ProducerInputRangeT>, GateT&>
;

template <class F, class GateT>
concept Consumer = std::invocable<F, thread_id_t, GateT&>;

template <producer_kind ProducerKind, consumer_kind ConsumerKind, ProducerInputRange ProducerInputRangeT, class T>
struct scheduler_traits
{
  static constexpr bool is_multi_push = ProducerKind == producer_kind::multi_push;
  static constexpr bool is_multi_pop = ConsumerKind == consumer_kind::multi_pop;

  using value_type = T;

  using producer_input_value_type = std::ranges::range_value_t<ProducerInputRangeT>;
  using producer_input_iterator = std::ranges::iterator_t<ProducerInputRangeT>;

  using queue_type = yk::concurrent_mpmc_vector<T>;

  using producer_gate_type = std::conditional_t<
    is_multi_push,
    yk::counted_info_producer_gate<queue_type>,
    yk::info_producer_gate<queue_type>
  >;

  using consumer_gate_type = std::conditional_t<
    is_multi_pop,
    yk::counted_info_consumer_gate<queue_type>,
    yk::info_consumer_gate<queue_type>
  >;
};

} // yk::exec

#endif
