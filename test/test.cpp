#include "yk/allocator/default_init_allocator.hpp"
#include "yk/hash.hpp"
#include "yk/hash/boost.hpp"
#include "yk/hash/hash_combine.hpp"
#include "yk/maybe_mutex.hpp"
#include "yk/par_for_each.hpp"
#include "yk/stack.hpp"
#include "yk/util/forward_like.hpp"
#include "yk/util/pack_indexing.hpp"
#include "yk/util/reverse.hpp"
#include "yk/util/specialization_of.hpp"
#include "yk/util/to_array_of.hpp"
#include "yk/util/to_subrange.hpp"
#include "yk/util/to_subrange/boost.hpp"

#define BOOST_TEST_MODULE yk_util_test
#include <boost/test/included/unit_test.hpp>

#include <boost/container_hash/hash.hpp>
#include <boost/range/iterator_range.hpp>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <exception>
#include <execution>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <version>

#if defined(__cpp_lib_forward_like) && !(defined(__clang__) && defined(__GLIBCXX__))
#define YK_UTIL_STD_HAS_FORWARD_LIKE 1
#else
#define YK_UTIL_STD_HAS_FORWARD_LIKE 0
#endif

namespace utf = boost::unit_test;

namespace hash_test {

template <class T, class... Ts>
struct S {
  int val;
};

struct MultiS {
  int a, b, c;
};

}  // namespace hash_test

YK_ADAPT_HASH_TEMPLATE(hash_test, (S<T, Ts...>), val, { return val.val; }, class T, class... Ts);
YK_ADAPT_HASH(hash_test, MultiS, val, { return yk::hash_combine(val.a, val.b, val.c); });

BOOST_AUTO_TEST_SUITE(yk_util)

BOOST_AUTO_TEST_CASE(PackIndexing) {
  static_assert(std::is_same_v<yk::pack_indexing_t<0, int, double, std::string>, int>);
  static_assert(std::is_same_v<yk::pack_indexing_t<1, int, double, std::string>, double>);
  static_assert(std::is_same_v<yk::pack_indexing_t<2, int, double, std::string>, std::string>);
}

BOOST_AUTO_TEST_CASE(SpecializationOf) {
  static_assert(yk::specialization_of<std::string, std::basic_string>);
  static_assert(yk::specialization_of<std::vector<int>, std::vector>);

  static_assert(!yk::specialization_of<std::string, std::vector>);
  static_assert(!yk::specialization_of<std::vector<int>, std::basic_string>);
}

#if YK_UTIL_STD_HAS_FORWARD_LIKE
template <class T, class U>
using std_fwd_like_t = decltype(std::forward_like<T>(std::declval<U>()));
#endif

BOOST_AUTO_TEST_CASE(ForwardLike) {
  // clang-format off
  static_assert(std::is_same_v<yk::forward_like_t<      int&, int  >,       int&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&, int& >,       int&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&, int&&>,       int&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&, int  >, const int&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&, int& >, const int&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&, int&&>, const int&>);

  static_assert(std::is_same_v<yk::forward_like_t<      int, int  >,       int&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int, int& >,       int&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int, int&&>,       int&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int, int  >, const int&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int, int& >, const int&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int, int&&>, const int&&>);

#if YK_UTIL_STD_HAS_FORWARD_LIKE
  static_assert(std::is_same_v<yk::forward_like_t<      int&, int  >, std_fwd_like_t<      int&, int  >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&, int& >, std_fwd_like_t<      int&, int& >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&, int&&>, std_fwd_like_t<      int&, int&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&, int  >, std_fwd_like_t<const int&, int  >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&, int& >, std_fwd_like_t<const int&, int& >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&, int&&>, std_fwd_like_t<const int&, int&&>>);

  static_assert(std::is_same_v<yk::forward_like_t<      int, int  >, std_fwd_like_t<      int, int  >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int, int& >, std_fwd_like_t<      int, int& >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int, int&&>, std_fwd_like_t<      int, int&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<const int, int  >, std_fwd_like_t<const int, int  >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int, int& >, std_fwd_like_t<const int, int& >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int, int&&>, std_fwd_like_t<const int, int&&>>);
#endif
  // clang-format on
}

BOOST_AUTO_TEST_CASE(ToArrayOf) {
  static_assert(yk::to_array_of<int>(3, 1, 4, 1, 5) == std::array{3, 1, 4, 1, 5});
  static_assert(yk::to_array_of<double>(3, 1, 4, 1, 5) == std::array{3.0, 1.0, 4.0, 1.0, 5.0});

  enum class StrongID : int {};
  constexpr auto predefined_ids = yk::to_array_of<StrongID>(1, 2, 3);
}

BOOST_AUTO_TEST_CASE(Reverse) {
  static_assert(yk::reverse(42) < yk::reverse(33 - 4));
  static_assert(yk::reverse(42) == yk::reverse(42));

  struct S {
    int a, b, c;
    constexpr bool operator==(const S&) const noexcept = default;
  };

  static_assert(std::ranges::equal(
      []() {
        std::vector<S> vec{{3, 1, 4}, {1, 5, 9}, {2, 6, 5}, {3, 5, 8}, {9, 7, 9}, {3, 2, 3}, {8, 4, 6}};
        std::ranges::sort(
            vec, [](const S& x, const S& y) { return std::forward_as_tuple(x.a, yk::reverse(x.b), x.c) < std::forward_as_tuple(y.a, yk::reverse(y.b), y.c); });
        return vec;
      }(),
      std::vector<S>{{1, 5, 9}, {2, 6, 5}, {3, 5, 8}, {3, 2, 3}, {3, 1, 4}, {8, 4, 6}, {9, 7, 9}}));

  static_assert(std::ranges::equal(
      []() {
        std::vector<S> vec{{3, 1, 4}, {1, 5, 9}, {2, 6, 5}, {3, 5, 8}, {9, 7, 9}, {3, 2, 3}, {8, 4, 6}};
        std::ranges::sort(vec, {}, [](const S& s) { return std::make_tuple(std::cref(s.a), yk::reverse(s.b), std::cref(s.c)); });
        return vec;
      }(),
      std::vector<S>{{1, 5, 9}, {2, 6, 5}, {3, 5, 8}, {3, 2, 3}, {3, 1, 4}, {8, 4, 6}, {9, 7, 9}}));
}

BOOST_AUTO_TEST_CASE(ToSubrange) {
  std::set<int> s{3, 1, 4, 1, 5};

  BOOST_TEST((std::ranges::equal(std::vector<int>{1}, yk::to_subrange(s.equal_range(1)))));
  BOOST_TEST((std::ranges::equal(std::vector<int>{}, yk::to_subrange(s.equal_range(2)))));

  std::vector v{3, 1, 4, 1, 5};
  auto rng = boost::make_iterator_range(v);
  BOOST_TEST((std::ranges::equal(v, yk::to_subrange(rng))));
}

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

BOOST_AUTO_TEST_CASE(Hash) {
  BOOST_TEST(std::hash<int>{}(42) == yk::std_hash_value_for(42));
  BOOST_TEST(boost::hash<int>{}(42) == yk::boost_hash_value_for(42));

  hash_test::S<int, double> s{42};
  BOOST_TEST(yk::hash_value_for(s) == yk::hash_value_for(42));
  BOOST_TEST(hash_value(s) == yk::hash_value_for(42));  // call hash_value by ADL

  {
    hash_test::MultiS s{31415, 9265, 3589};
    std::size_t seed = yk::hash_value_for(s.a);
    boost::hash_combine(seed, yk::hash_value_for(s.b));
    boost::hash_combine(seed, yk::hash_value_for(s.c));
    BOOST_TEST(hash_value(s) == seed);
  }
}

BOOST_AUTO_TEST_SUITE_END()  // yk_util
