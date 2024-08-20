#include "yk/util/forward_like.hpp"
#include "yk/util/pack_indexing.hpp"
#include "yk/util/reverse.hpp"
#include "yk/util/specialization_of.hpp"
#include "yk/util/to_array_of.hpp"
#include "yk/util/to_subrange.hpp"
#include "yk/util/to_subrange/boost.hpp"

#define BOOST_TEST_MODULE yk_util_test
#include <boost/test/included/unit_test.hpp>

#include <boost/range/iterator_range.hpp>

#include <functional>
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

BOOST_AUTO_TEST_SUITE_END()  // yk_util
