#include "yk/concurrent_deque.hpp"
#include "yk/concurrent_vector.hpp"
#include "yk/maybe_mutex.hpp"
#include "yk/par_for_each.hpp"

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <atomic>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <stop_token>
#include <thread>
#include <vector>
#include <version>

BOOST_AUTO_TEST_SUITE(concurrent)

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

BOOST_AUTO_TEST_CASE(ConcurrentVector) {
  // stack-like pool
  {
    {
      yk::concurrent_vector<int> pool;
      BOOST_REQUIRE_THROW(pool.set_capacity(-1), std::length_error);
    }
    {
      yk::concurrent_vector<int> pool;
      BOOST_REQUIRE_NO_THROW(pool.set_capacity(1024));
      BOOST_REQUIRE_NO_THROW(pool.reserve_capacity());
    }
    {
      yk::concurrent_vector<int> stack_like_pool;
      (void)stack_like_pool.push_wait(1);
      (void)stack_like_pool.push_wait(2);
      int value = -1;
      (void)stack_like_pool.pop_wait(value);
      BOOST_TEST(value == 2);
      (void)stack_like_pool.pop_wait(value);
      BOOST_TEST(value == 1);
    }
    // Single Producer Single Consumer
    {
      using CV = yk::concurrent_spsc_vector<int>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          (void)vec.push_wait(i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          result.push_back(value);
        }
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
          (void)vec.push_wait(id * 100 + i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10 * 4; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          result.push_back(value);
        }
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
          (void)vec.push_wait(i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
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
          (void)vec.push_wait(id * 100 + i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
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
          (void)vec.push_wait(i);
        }
      };
      std::vector<int> result;
      const auto consumer = [&](CV& vec, std::stop_token stoken) {
        while (true) {
          if (stoken.stop_requested()) break;
          int value = -1;
          try {
            (void)vec.pop_wait(stoken, value);
          } catch (yk::interrupt_exception&) {
            break;
          }
          result.push_back(value);
        }
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
      (void)stack_like_pool.push_wait(1);
      (void)stack_like_pool.push_wait(2);
      int value = -1;
      (void)stack_like_pool.pop_wait(value);
      BOOST_TEST(value == 2);
      (void)stack_like_pool.pop_wait(value);
      BOOST_TEST(value == 1);
    }
    // Single Producer Single Consumer
    {
      using CV = yk::concurrent_spsc_deque<int>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          (void)vec.push_wait(i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          result.push_back(value);
        }
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
      using CV = yk::concurrent_mpsc_deque<int>;
      const auto producer = [](CV& vec, int id) {
        for (int i = 0; i < 10; ++i) {
          (void)vec.push_wait(id * 100 + i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10 * 4; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          result.push_back(value);
        }
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
      using CV = yk::concurrent_deque<int, yk::concurrent_pool_flag::spmc>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 40; ++i) {
          (void)vec.push_wait(i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
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
      using CV = yk::concurrent_mpmc_deque<int>;
      auto producer = [](CV& vec, int id) {
        for (int i = 0; i < 10; ++i) {
          (void)vec.push_wait(id * 100 + i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
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
      using CV = yk::concurrent_spsc_deque<int, yk::concurrent_pool_flag::stop_token_support>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          (void)vec.push_wait(i);
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
            (void)vec.pop_wait(stoken, value);
          } catch (yk::interrupt_exception&) {
            break;
          }
          result.push_back(value);
        }
      };
      CV vec;
      std::stop_source ssource;
      std::thread producer_thread(producer, std::ref(vec));
      std::thread consumer_thread(consumer, std::ref(vec), ssource.get_token());
      producer_thread.join();
      ssource.request_stop();
      consumer_thread.join();
      std::ranges::sort(result);
      BOOST_TEST((std::ranges::includes(std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, result)));
    }
#endif
  }
  // queue-like pool
  {
    {
      yk::concurrent_deque<int, yk::concurrent_pool_flag::queue_based_push_pop> queue_like_pool;
      (void)queue_like_pool.push_wait(1);
      (void)queue_like_pool.push_wait(2);
      int value = -1;
      (void)queue_like_pool.pop_wait(value);
      BOOST_TEST(value == 1);
      (void)queue_like_pool.pop_wait(value);
      BOOST_TEST(value == 2);
    }
    // Single Producer Single Consumer
    {
      using CV = yk::concurrent_spsc_deque<int, yk::concurrent_pool_flag::queue_based_push_pop>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          (void)vec.push_wait(i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          result.push_back(value);
        }
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
      using CV = yk::concurrent_mpsc_deque<int, yk::concurrent_pool_flag::queue_based_push_pop>;
      const auto producer = [](CV& vec, int id) {
        for (int i = 0; i < 10; ++i) {
          (void)vec.push_wait(id * 100 + i);
        }
      };

      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10 * 4; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          result.push_back(value);
        }
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
      using CV = yk::concurrent_spmc_deque<int, yk::concurrent_pool_flag::queue_based_push_pop>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 40; ++i) {
          (void)vec.push_wait(i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
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
      using CV = yk::concurrent_mpmc_deque<int, yk::concurrent_pool_flag::queue_based_push_pop>;
      auto producer = [](CV& vec, int id) {
        for (int i = 0; i < 10; ++i) {
          (void)vec.push_wait(id * 100 + i);
        }
      };
      std::mutex mtx;
      std::vector<int> result;
      const auto consumer = [&](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          int value = -1;
          (void)vec.pop_wait(value);
          std::lock_guard lock(mtx);
          result.push_back(value);
        }
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
      using CV = yk::concurrent_spsc_deque<int, yk::concurrent_pool_flag::stop_token_support | yk::concurrent_pool_flag::queue_based_push_pop>;
      const auto producer = [](CV& vec) {
        for (int i = 0; i < 10; ++i) {
          (void)vec.push_wait(i);
        }
      };
      std::vector<int> result;
      const auto consumer = [&](CV& vec, std::stop_token stoken) {
        while (true) {
          if (stoken.stop_requested()) break;
          int value = -1;
          try {
            (void)vec.pop_wait(stoken, value);
          } catch (yk::interrupt_exception&) {
            break;
          }
          result.push_back(value);
        }
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

BOOST_AUTO_TEST_SUITE_END()
