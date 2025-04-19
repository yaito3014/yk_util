#include "yk/allocator/default_init_allocator.hpp"
#include "yk/concurrent_deque.hpp"
#include "yk/concurrent_vector.hpp"
#include "yk/maybe_mutex.hpp"
#include "yk/par_for_each.hpp"
#include "yk/ranges/concat.hpp"
#include "yk/stack.hpp"

#define BOOST_TEST_MODULE yk_util_test
#if YK_BUILD_UNIT_TEST_FRAMEWORK
#include <boost/test/included/unit_test.hpp>
#else
#include <boost/test/unit_test.hpp>
#endif

#include <boost/range/iterator_range.hpp>

#include <algorithm>
#include <atomic>
#include <compare>
#include <execution>
#include <forward_list>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <ranges>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <utility>
#include <vector>
#include <version>

#include <cstdint>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(yk_util)

BOOST_AUTO_TEST_CASE(Allocator) {
  {
    unsigned char uninitialized_storage[sizeof(std::uint32_t)] = {0xde, 0xad, 0xbe, 0xef};
    alignas(std::uint32_t) unsigned char storage[sizeof(std::uint32_t)];
    std::ranges::copy(uninitialized_storage, storage);

    std::allocator<std::uint32_t> alloc;
    std::allocator_traits<std::allocator<std::uint32_t>>::construct(alloc, reinterpret_cast<std::uint32_t*>(storage));

    BOOST_TEST(!std::ranges::equal(uninitialized_storage, storage));
    BOOST_TEST(std::ranges::equal(std::vector<unsigned char>{0, 0, 0, 0}, storage));
  }
  {
    unsigned char uninitialized_storage[sizeof(std::uint32_t)] = {0xde, 0xad, 0xbe, 0xef};
    alignas(std::uint32_t) unsigned char storage[sizeof(std::uint32_t)];
    std::ranges::copy(uninitialized_storage, storage);

    yk::default_init_allocator<std::uint32_t> alloc;
    alloc.construct(storage);

    BOOST_TEST(std::ranges::equal(uninitialized_storage, storage));
  }
}

BOOST_AUTO_TEST_CASE(Stack) {
  yk::stack<int> s{3, 1, 4, 1, 5};

  s.shrink_to_fit();
  BOOST_TEST(s.capacity() == 5);
  s.reserve(10);
  BOOST_TEST(s.capacity() >= 10);
  BOOST_TEST(s.capacity() <= s.max_size());

  BOOST_TEST(!s.empty());
  s.clear();
  BOOST_TEST(s.empty());
}

#if __cpp_lib_parallel_algorithm >= 201603L

BOOST_AUTO_TEST_CASE(ParForEach) {
  std::vector vec{3, 1, 4, 1, 5};
  std::atomic<int> sum;
  const auto non_throw_func = [&](int x) { sum += x; };
  const auto throw_func = [&](int x) {
    if (x == 4) throw std::runtime_error("");
    sum += x;
  };

  std::for_each(std::execution::seq, vec.begin(), vec.end(), non_throw_func);

  {
    sum = 0;
    yk::ranges::for_each(yk::execution::abort, std::execution::seq, vec, non_throw_func);
    BOOST_TEST((sum == 14));
  }
  {
    sum = 0;
    BOOST_REQUIRE_THROW(yk::ranges::for_each(yk::execution::abort, std::execution::seq, vec, throw_func), std::runtime_error);
    BOOST_TEST((sum < 14));
  }
  {
    sum = 0;
    yk::ranges::for_each(yk::execution::proceed, std::execution::seq, vec, non_throw_func);
    BOOST_TEST((sum == 14));
  }
  {
    sum = 0;
    BOOST_REQUIRE_THROW(yk::ranges::for_each(yk::execution::proceed, std::execution::seq, vec, throw_func), std::runtime_error);
    BOOST_TEST((sum == 10));
  }
}

BOOST_AUTO_TEST_CASE(MaybeMutex) {
  static_assert(yk::xo::Lockable<yk::maybe_mutex<std::mutex, std::execution::parallel_policy>>);
  static_assert(yk::xo::Lockable<yk::maybe_mutex<std::mutex, std::execution::sequenced_policy>>);
}

#endif  // __cpp_lib_parallel_algorithm

BOOST_AUTO_TEST_CASE(Concat) {
  using namespace std::literals;
  {
    auto rng = yk::views::concat("foo"sv, "bar"sv, "baz"sv);
    static_assert(std::ranges::random_access_range<decltype(rng)>);
    static_assert(std::ranges::view<decltype(rng)>);
    std::random_access_iterator auto iter = std::ranges::begin(rng);
    std::random_access_iterator auto sent = std::ranges::end(rng);
    BOOST_TEST((iter != sent));
    BOOST_TEST((sent - iter) == 9);
    BOOST_TEST((iter == iter));
    BOOST_TEST((iter - iter) == 0);
    BOOST_TEST((std::default_sentinel - iter) == 9);
    BOOST_TEST((iter - std::default_sentinel) == -9);
    BOOST_TEST(*(iter + 4) == 'a');
    BOOST_TEST(((iter + 1 + 1 + 1) - 3 == iter));
    BOOST_TEST(iter[2] == 'o');
    BOOST_TEST(iter[3] == 'b');
    BOOST_TEST(iter[4] == 'a');
    BOOST_TEST(rng[2] == 'o');
    BOOST_TEST(rng[3] == 'b');
    BOOST_TEST(rng[4] == 'a');
    BOOST_TEST(std::ranges::size(rng) == rng.size());
    BOOST_TEST(std::ranges::size(rng) == 9);
    BOOST_TEST(std::ranges::equal(rng, "foobarbaz"sv));
    {
      auto i = iter, j = iter;
      BOOST_TEST((++j == i + 1));
      BOOST_TEST((--j == i));

      BOOST_TEST((j++ == i));
      BOOST_TEST((j == i + 1));

      BOOST_TEST((j-- == i + 1));
      BOOST_TEST((j == i));

      BOOST_TEST(((j += 3) == i + 3));
      BOOST_TEST(((j -= 2) == i + 1));

      BOOST_TEST((i < j));
      BOOST_TEST((i <= j));
      BOOST_TEST((j > i));
      BOOST_TEST((j >= i));
      BOOST_TEST(((i <=> j) < 0));
    }
  }
  {
    std::vector<int> vec1;  // random_access_range
    std::vector<int> vec2;  // random_access_range
    auto rng = yk::views::concat(vec1, vec2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::random_access_range<decltype(rng)>);  // concat-ing contiguous_ranges never be contiguous_range
  }
  {
    std::list<int> list1;  // bidirectional_range
    std::list<int> list2;  // bidirectional_range
    auto rng = yk::views::concat(list1, list2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::bidirectional_range<decltype(rng)>);
  }
  {
    std::forward_list<int> forward_list1;  // forward_range
    std::forward_list<int> forward_list2;  // forward_range
    auto rng = yk::views::concat(forward_list1, forward_list2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::forward_range<decltype(rng)>);
  }
  {
    auto input1 = std::views::istream<int>(std::cin);  // input_range
    auto input2 = std::views::istream<int>(std::cin);  // input_range
    auto rng = yk::views::concat(input1, input2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::input_range<decltype(rng)>);
  }
  {
    auto sized1 = std::views::iota(0, 1);  // sized_range
    auto sized2 = std::views::iota(0, 1);  // sized_range
    auto rng = yk::views::concat(sized1, sized2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::sized_range<decltype(rng)>);
  }
  {
    auto&& vec1 = std::vector<int>{};  // viewable_range
    auto&& vec2 = std::vector<int>{};  // viewable_range
    auto rng = yk::views::concat(std::move(vec1), std::move(vec2));
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::viewable_range<decltype(std::move(rng))>);
  }
#if __cpp_lib_ranges_as_const >= 202207L
  {
    auto constant1 = std::views::iota(0, 1);  // constant_range
    auto constant2 = std::views::iota(0, 1);  // constant_range
    auto rng = yk::views::concat(constant1, constant2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::constant_range<decltype(rng)>);
  }
#endif
  {
    auto not_common = std::views::iota(0);  // arbitrary range
    auto common = std::views::iota(0, 1);   // common_range
    auto rng = yk::views::concat(not_common, common);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::common_range<decltype(rng)>);
  }
  {
    auto view = std::views::istream<int>(std::cin);  // non-common_range with non-copyable iterator
    std::vector<int> vec;                            // common_range with copyable iterator
    auto rng = yk::views::concat(view, vec);
    static_assert(std::ranges::range<decltype(rng)>);  // ill-formed as of P2542R8, but well-formed in our implementation
    // static_assert(std::ranges::common_range<decltype(rng)>);  // in theory, this should be true if above is well-formed, but ill-formed in our implementation
  }
  {
    struct InputIterator {
      using value_type = int;
      using difference_type = int;
      int operator*() const { return 0; }
      InputIterator& operator++() { return *this; }
      InputIterator operator++(int) { return *this; }
    };
    auto input_range = std::ranges::subrange(InputIterator{}, std::unreachable_sentinel);
    static_assert(std::ranges::input_range<decltype(input_range)>);
    auto concat = yk::views::concat(input_range, std::views::iota(0, 1));
    static_assert(std::ranges::range<decltype(concat)>);
  }
  {
    std::vector<int> vec;  // random_access_range
    std::list<int> list;   // bidirectional_range
    auto rng = yk::views::concat(vec, list);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::bidirectional_range<decltype(rng)>);
  }
  {
    std::vector<int> vec;                 // random_access_range
    std::forward_list<int> forward_list;  // forward_range
    auto rng = yk::views::concat(vec, forward_list);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::forward_range<decltype(rng)>);
  }
  {
    std::vector<int> vec;                            // random_access_range
    auto view = std::views::istream<int>(std::cin);  // input_range
    auto rng = yk::views::concat(vec, view);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::input_range<decltype(rng)>);
  }

  {
    std::vector a{3, 1, 4, 1, 5};
    std::vector b{9, 2, 6, 5};
    auto view = yk::views::concat(a, b);
    std::ranges::sort(view);
    BOOST_TEST(std::ranges::equal(view, std::vector{1, 1, 2, 3, 4, 5, 5, 6, 9}));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    std::vector b{9, 2, 6, 5};
    std::vector c{3, 5, 8, 9, 7, 9};
    auto view = yk::views::concat(yk::views::concat(a, b), c);
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9}));
  }
  {
    auto view = yk::views::concat(std::views::empty<int>, std::views::empty<int>, std::views::empty<int>);
    BOOST_TEST(std::ranges::empty(view));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    auto view = yk::views::concat(std::views::empty<int>, a, std::views::empty<int>);
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 4, 1, 5}));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    std::vector b{9, 2, 6, 5};
    auto view = yk::views::concat(a, std::views::empty<int>, b);
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 4, 1, 5, 9, 2, 6, 5}));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    auto view = yk::views::concat(a, a);
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 4, 1, 5, 3, 1, 4, 1, 5}));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    auto view = yk::views::concat(a | std::views::take(3), a | std::views::drop(3));
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 4, 1, 5}));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    auto view = yk::views::concat(a | std::views::take(2), a | std::views::drop(1));
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 1, 4, 1, 5}));
  }
  {
    auto a = yk::views::concat("pen"sv, "pineapple"sv);
    auto b = yk::views::concat("apple"sv, "pen"sv);
    BOOST_TEST(std::ranges::equal(yk::views::concat(a, b), "penpineappleapplepen"sv));
  }
  {
    int a[]{2, 7, 1, 8, 2, 8};
    int b[]{1, 4, 1, 4, 2};
    BOOST_TEST(std::ranges::equal(yk::views::concat(a, b), std::vector{2, 7, 1, 8, 2, 8, 1, 4, 1, 4, 2}));
  }
}
BOOST_AUTO_TEST_CASE(ConcurrentVector) {
  // stack-like pool
  {
    {
      yk::concurrent_vector<int> pool;
      BOOST_REQUIRE_THROW(pool.reserve(-1), std::length_error);
    }
    {
      yk::concurrent_vector<int> stack_like_pool;
      stack_like_pool.push_wait(1);
      stack_like_pool.push_wait(2);
      int value = -1;
      stack_like_pool.pop_wait(value);
      BOOST_TEST(value == 2);
      stack_like_pool.pop_wait(value);
      BOOST_TEST(value == 1);
    }
    // Single Producer Single Consumer
    {
      using CV = yk::concurrent_spsc_vector<int>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          vec.pop_wait(value);
          result.push_back(value);
        }
        return result;
      };

      CV vec;
      std::thread producer_thread(producer, std::ref(vec));
      std::thread consumer_thread(consumer, std::ref(vec));
      producer_thread.join();
      consumer_thread.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }
    // Multi Producer Single Consumer
    {
      using CV = yk::concurrent_mpsc_vector<int>;
      const auto producer = [](CV& vec, int id) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(id * 100 + i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10 * 4; ++i) {
          int value = -1;
          vec.pop_wait(value);
          result.push_back(value);
        }
        return result;
      };

      CV vec;
      std::thread producer_thread1(producer, std::ref(vec), 0);
      std::thread producer_thread2(producer, std::ref(vec), 1);
      std::thread producer_thread3(producer, std::ref(vec), 2);
      std::thread producer_thread4(producer, std::ref(vec), 3);
      std::thread consumer_thread(consumer, std::ref(vec));
      producer_thread1.join();
      producer_thread2.join();
      producer_thread3.join();
      producer_thread4.join();
      consumer_thread.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::vector<int>{
                                                0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
                                                200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
                                            }));
    }
    // Single Producer Multi Consumer
    {
      using CV = yk::concurrent_vector<int, yk::concurrent_pool_flag::spmc>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 40; ++i) {
          vec.push_wait(i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
        return result;
      };
      CV vec;
      std::thread producer_thread(producer, std::ref(vec));
      std::thread consumer_thread1(consumer, std::ref(vec));
      std::thread consumer_thread2(consumer, std::ref(vec));
      std::thread consumer_thread3(consumer, std::ref(vec));
      std::thread consumer_thread4(consumer, std::ref(vec));
      producer_thread.join();
      consumer_thread1.join();
      consumer_thread2.join();
      consumer_thread3.join();
      consumer_thread4.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::vector<int>{
                                                0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                                                20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                                            }));
    }
    // Multi Producer Multi Consumer
    {
      using CV = yk::concurrent_mpmc_vector<int>;
      auto producer = [](CV& vec, int id) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(id * 100 + i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
        return result;
      };
      CV vec;
      std::thread producer_thread1(producer, std::ref(vec), 0);
      std::thread producer_thread2(producer, std::ref(vec), 1);
      std::thread producer_thread3(producer, std::ref(vec), 2);
      std::thread producer_thread4(producer, std::ref(vec), 3);
      std::thread consumer_thread1(consumer, std::ref(vec));
      std::thread consumer_thread2(consumer, std::ref(vec));
      std::thread consumer_thread3(consumer, std::ref(vec));
      std::thread consumer_thread4(consumer, std::ref(vec));
      producer_thread1.join();
      producer_thread2.join();
      producer_thread3.join();
      producer_thread4.join();
      consumer_thread1.join();
      consumer_thread2.join();
      consumer_thread3.join();
      consumer_thread4.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::vector<int>{
                                                0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
                                                200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
                                            }));
    }
#if __cpp_lib_jthread >= 201911L
    // stop_token
    {
      using CV = yk::concurrent_spsc_vector<int, yk::concurrent_pool_flag::stop_token_support>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(i);
        }
      };
      std::vector<int> result;
      const auto consumer = [&](CV& vec, std::stop_token stoken) {
        while (true) {
          if (stoken.stop_requested()) break;
          int value = -1;
          try {
            vec.pop_wait(value, stoken);
          } catch (yk::interrupt_exception&) {
            break;
          }
          result.push_back(value);
        }
        return result;
      };
      CV vec;
      std::thread producer_thread(producer, std::ref(vec));
      std::stop_source ssource;
      std::thread consumer_thread(consumer, std::ref(vec), ssource.get_token());
      producer_thread.join();
      ssource.request_stop();
      consumer_thread.join();
      std::ranges::sort(result);
      BOOST_TEST((std::ranges::includes(std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, result)));
    }
#endif
  }
}
BOOST_AUTO_TEST_CASE(ConcurrentDeque) {
  // stack-like pool
  {
    {
      yk::concurrent_deque<int> stack_like_pool;
      stack_like_pool.push_wait(1);
      stack_like_pool.push_wait(2);
      int value = -1;
      stack_like_pool.pop_wait(value);
      BOOST_TEST(value == 2);
      stack_like_pool.pop_wait(value);
      BOOST_TEST(value == 1);
    }
    // Single Producer Single Consumer
    {
      using CV = yk::concurrent_spsc_deque<int>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          vec.pop_wait(value);
          result.push_back(value);
        }
        return result;
      };

      CV vec;
      std::thread producer_thread(producer, std::ref(vec));
      std::thread consumer_thread(consumer, std::ref(vec));
      producer_thread.join();
      consumer_thread.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::deque<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }
    // Multi Producer Single Consumer
    {
      using CV = yk::concurrent_mpsc_deque<int>;
      const auto producer = [](CV& vec, int id) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(id * 100 + i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10 * 4; ++i) {
          int value = -1;
          vec.pop_wait(value);
          result.push_back(value);
        }
        return result;
      };

      CV vec;
      std::thread producer_thread1(producer, std::ref(vec), 0);
      std::thread producer_thread2(producer, std::ref(vec), 1);
      std::thread producer_thread3(producer, std::ref(vec), 2);
      std::thread producer_thread4(producer, std::ref(vec), 3);
      std::thread consumer_thread(consumer, std::ref(vec));
      producer_thread1.join();
      producer_thread2.join();
      producer_thread3.join();
      producer_thread4.join();
      consumer_thread.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::deque<int>{
                                                0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
                                                200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
                                            }));
    }
    // Single Producer Multi Consumer
    {
      using CV = yk::concurrent_deque<int, yk::concurrent_pool_flag::spmc>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 40; ++i) {
          vec.push_wait(i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
        return result;
      };
      CV vec;
      std::thread producer_thread(producer, std::ref(vec));
      std::thread consumer_thread1(consumer, std::ref(vec));
      std::thread consumer_thread2(consumer, std::ref(vec));
      std::thread consumer_thread3(consumer, std::ref(vec));
      std::thread consumer_thread4(consumer, std::ref(vec));
      producer_thread.join();
      consumer_thread1.join();
      consumer_thread2.join();
      consumer_thread3.join();
      consumer_thread4.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::deque<int>{
                                                0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                                                20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                                            }));
    }
    // Multi Producer Multi Consumer
    {
      using CV = yk::concurrent_mpmc_deque<int>;
      auto producer = [](CV& vec, int id) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(id * 100 + i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
        return result;
      };
      CV vec;
      std::thread producer_thread1(producer, std::ref(vec), 0);
      std::thread producer_thread2(producer, std::ref(vec), 1);
      std::thread producer_thread3(producer, std::ref(vec), 2);
      std::thread producer_thread4(producer, std::ref(vec), 3);
      std::thread consumer_thread1(consumer, std::ref(vec));
      std::thread consumer_thread2(consumer, std::ref(vec));
      std::thread consumer_thread3(consumer, std::ref(vec));
      std::thread consumer_thread4(consumer, std::ref(vec));
      producer_thread1.join();
      producer_thread2.join();
      producer_thread3.join();
      producer_thread4.join();
      consumer_thread1.join();
      consumer_thread2.join();
      consumer_thread3.join();
      consumer_thread4.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::deque<int>{
                                                0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
                                                200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
                                            }));
    }
#if __cpp_lib_jthread >= 201911L
    // stop_token
    {
      using CV = yk::concurrent_spsc_deque<int, yk::concurrent_pool_flag::stop_token_support>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(i);
        }
      };
      std::vector<int> result;
      const auto consumer = [&](CV& vec, std::stop_token stoken) {
        while (true) {
          if (stoken.stop_requested()) {
            break;
          }
          int value = -1;
          try {
            vec.pop_wait(value, stoken);
          } catch (yk::interrupt_exception&) {
            break;
          }
          result.push_back(value);
        }
        return result;
      };
      CV vec;
      std::stop_source ssource;
      std::thread producer_thread(producer, std::ref(vec));
      std::thread consumer_thread(consumer, std::ref(vec), ssource.get_token());
      producer_thread.join();
      ssource.request_stop();
      consumer_thread.join();
      std::ranges::sort(result);
      BOOST_TEST((std::ranges::includes(std::deque<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, result)));
    }
#endif
  }
  // queue-like pool
  {
    {
      yk::concurrent_deque<int, yk::concurrent_pool_flag::queue_based_push_pop> queue_like_pool;
      queue_like_pool.push_wait(1);
      queue_like_pool.push_wait(2);
      int value = -1;
      queue_like_pool.pop_wait(value);
      BOOST_TEST(value == 1);
      queue_like_pool.pop_wait(value);
      BOOST_TEST(value == 2);
    }
    // Single Producer Single Consumer
    {
      using CV = yk::concurrent_spsc_deque<int, yk::concurrent_pool_flag::queue_based_push_pop>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          vec.pop_wait(value);
          result.push_back(value);
        }
        return result;
      };

      CV vec;
      std::thread producer_thread(producer, std::ref(vec));
      std::thread consumer_thread(consumer, std::ref(vec));
      producer_thread.join();
      consumer_thread.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::deque<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));
    }
    // Multi Producer Single Consumer
    {
      using CV = yk::concurrent_mpsc_deque<int, yk::concurrent_pool_flag::queue_based_push_pop>;
      const auto producer = [](CV& vec, int id) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(id * 100 + i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10 * 4; ++i) {
          int value = -1;
          vec.pop_wait(value);
          result.push_back(value);
        }
        return result;
      };

      CV vec;
      std::thread producer_thread1(producer, std::ref(vec), 0);
      std::thread producer_thread2(producer, std::ref(vec), 1);
      std::thread producer_thread3(producer, std::ref(vec), 2);
      std::thread producer_thread4(producer, std::ref(vec), 3);
      std::thread consumer_thread(consumer, std::ref(vec));
      producer_thread1.join();
      producer_thread2.join();
      producer_thread3.join();
      producer_thread4.join();
      consumer_thread.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::deque<int>{
                                                0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
                                                200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
                                            }));
    }
    // Single Producer Multi Consumer
    {
      using CV = yk::concurrent_spmc_deque<int, yk::concurrent_pool_flag::queue_based_push_pop>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 40; ++i) {
          vec.push_wait(i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
        return result;
      };
      CV vec;
      std::thread producer_thread(producer, std::ref(vec));
      std::thread consumer_thread1(consumer, std::ref(vec));
      std::thread consumer_thread2(consumer, std::ref(vec));
      std::thread consumer_thread3(consumer, std::ref(vec));
      std::thread consumer_thread4(consumer, std::ref(vec));
      producer_thread.join();
      consumer_thread1.join();
      consumer_thread2.join();
      consumer_thread3.join();
      consumer_thread4.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::deque<int>{
                                                0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                                                20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                                            }));
    }
    // Multi Producer Multi Consumer
    {
      using CV = yk::concurrent_mpmc_deque<int, yk::concurrent_pool_flag::queue_based_push_pop>;
      auto producer = [](CV& vec, int id) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(id * 100 + i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
        return result;
      };
      CV vec;
      std::thread producer_thread1(producer, std::ref(vec), 0);
      std::thread producer_thread2(producer, std::ref(vec), 1);
      std::thread producer_thread3(producer, std::ref(vec), 2);
      std::thread producer_thread4(producer, std::ref(vec), 3);
      std::thread consumer_thread1(consumer, std::ref(vec));
      std::thread consumer_thread2(consumer, std::ref(vec));
      std::thread consumer_thread3(consumer, std::ref(vec));
      std::thread consumer_thread4(consumer, std::ref(vec));
      producer_thread1.join();
      producer_thread2.join();
      producer_thread3.join();
      producer_thread4.join();
      consumer_thread1.join();
      consumer_thread2.join();
      consumer_thread3.join();
      consumer_thread4.join();
      std::ranges::sort(result);
      BOOST_TEST(std::ranges::equal(result, std::deque<int>{
                                                0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
                                                200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
                                            }));
    }
#if __cpp_lib_jthread >= 201911L
    // stop_token
    {
      using CV = yk::concurrent_spsc_deque<int, yk::concurrent_pool_flag::stop_token_support | yk::concurrent_pool_flag::queue_based_push_pop>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          vec.push_wait(i);
        }
      };
      std::vector<int> result;
      const auto consumer = [&](CV& vec, std::stop_token stoken) {
        while (true) {
          if (stoken.stop_requested()) break;
          int value = -1;
          try {
            vec.pop_wait(value, stoken);
          } catch (yk::interrupt_exception&) {
            break;
          }
          result.push_back(value);
        }
        return result;
      };
      CV vec;
      std::thread producer_thread(producer, std::ref(vec));
      std::stop_source ssource;
      std::thread consumer_thread(consumer, std::ref(vec), ssource.get_token());
      producer_thread.join();
      ssource.request_stop();
      consumer_thread.join();
      std::ranges::sort(result);
      BOOST_TEST((std::ranges::includes(std::deque<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, result)));
    }
#endif
  }
}

BOOST_AUTO_TEST_SUITE_END()  // yk_util
