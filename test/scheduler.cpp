#define YK_EXEC_DEBUG_PRINT(...) do {} while (false)

#include "yk/exec/scheduler.hpp"
#include "yk/exec/atomic_queue.hpp"

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <print>
#include <format>
#include <string>
#include <vector>
#include <tuple>
#include <ranges>
#include <mutex>
#include <memory>
#include <type_traits>

namespace {
} // anon

BOOST_AUTO_TEST_SUITE(scheduler)

BOOST_AUTO_TEST_CASE(example)
{
  auto worker_pool = std::make_shared<yk::exec::worker_pool>();
  worker_pool->set_worker_limit(8);


  // emulate std::views::cartesian_product (which does not exist in libc++)
  constexpr unsigned long long ID_A_MAX = 100;
  constexpr unsigned long long ID_B_MAX = 100;
  constexpr unsigned long long ID_C_MAX = 10;
  constexpr unsigned long long ID_COUNT = ID_A_MAX * ID_B_MAX * ID_C_MAX;
  using ID = std::tuple<unsigned long long, unsigned long long, unsigned long long>;

  std::mutex mtx;
  std::vector<ID> results;
  results.reserve(ID_COUNT);

  auto sched = yk::exec::make_scheduler<
    yk::exec::producer_kind::multi_push, yk::exec::consumer_kind::multi_pop,
    yk::exec::atomic_queue<ID>
  >(
    worker_pool,

    // Producer
    [](yk::exec::thread_index_t /*thread_id*/, unsigned long long id_int, auto& queue) {
      const auto id_c = id_int % ID_C_MAX;
      id_int /= ID_C_MAX;
      const auto id_b = id_int % ID_B_MAX;
      id_int /= ID_B_MAX;
      const auto id_a = id_int % ID_A_MAX;

      ID id = std::make_tuple(id_a, id_b, id_c);
      if (!queue.push_wait(std::move(id))) return;
    },

    // Consumer
    [&](yk::exec::thread_index_t /*thread_id*/, auto& queue) {
      ID id;
      if (!queue.pop_wait(id)) return;

      std::scoped_lock lock{mtx};
      results.emplace_back(std::move(id));
    },

    // Producer input range (can be any forward_range)
    std::views::iota(0ull, ID_COUNT),

    // queue capacity (= constructor arguments passed to the underlying queue)
    10 * 1024
  );

  sched.set_producer_chunk_size(100);

  BOOST_REQUIRE_NO_THROW(sched.start());
  BOOST_REQUIRE_NO_THROW(sched.wait_for_all_tasks());

  BOOST_REQUIRE(results.size() == ID_COUNT);
  std::ranges::sort(results);

  std::vector<ID> expected_results;
  expected_results.reserve(ID_COUNT);

  for (unsigned long long id_a = 0; id_a < ID_A_MAX; ++id_a) {
    for (unsigned long long id_b = 0; id_b < ID_B_MAX; ++id_b) {
      for (unsigned long long id_c = 0; id_c < ID_C_MAX; ++id_c) {
        expected_results.emplace_back(std::make_tuple(id_a, id_b, id_c));
      }
    }
  }

  BOOST_TEST(std::ranges::equal(results, expected_results) == true);
}

BOOST_AUTO_TEST_SUITE_END()
