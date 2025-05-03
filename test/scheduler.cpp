#include "yk/exec/scheduler.hpp"
#include "yk/exec/atomic_queue.hpp"

#include <boost/test/unit_test.hpp>

#include <print>
#include <format>
#include <tuple>
#include <ranges>
#include <memory>
#include <type_traits>

namespace {

struct MyData
{
  std::string name;
};

} // anon

BOOST_AUTO_TEST_SUITE(scheduler)

BOOST_AUTO_TEST_CASE(example)
{
  auto worker_pool = std::make_shared<yk::exec::worker_pool>();
  worker_pool->set_worker_limit(8);

  const auto ids = std::views::cartesian_product(
    std::views::iota(0, 10),
    std::views::iota(0, 10),
    std::views::iota(0, 10)
  );

  auto sched = yk::exec::make_scheduler<
    yk::exec::producer_kind::multi_push, yk::exec::consumer_kind::multi_pop,
    yk::exec::atomic_queue<MyData>
  >(
    worker_pool,

    // Producer
    [](yk::exec::thread_index_t /*thread_id*/, const std::tuple<int, int, int>& id, auto& queue) {
        MyData data;
        data.name = std::format("{}_{}_{}", std::get<0>(id), std::get<1>(id), std::get<2>(id));
        if (!queue.push_wait(std::move(data))) return;
    },

    // Consumer
    [](yk::exec::thread_index_t /*thread_id*/, auto& queue) {
        MyData data;
        if (!queue.pop_wait(data)) return;
        // ...
        // ...
        // do_something(data);
        // ...
        // ...
    },

    // Producer input range (can be any forward_range)
    ids,

    // queue capacity (= constructor arguments passed to the underlying queue)
    10 * 1024
  );

  sched.set_producer_chunk_size(100);

  std::println("starting simulation...");
  BOOST_REQUIRE_NO_THROW(sched.start());
  BOOST_REQUIRE_NO_THROW(sched.wait_for_all_tasks());
  std::println("simulation done!");
}

BOOST_AUTO_TEST_SUITE_END()
