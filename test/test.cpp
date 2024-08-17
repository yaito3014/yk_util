#include "yk/util/forward_like.hpp"
#include "yk/util/pack_indexing.hpp"
#include "yk/util/specialization_of.hpp"
#include "yk/util/to_array_of.hpp"

#define BOOST_TEST_MODULE yk_util_test
#include <boost/test/unit_test.hpp>

#include <string>
#include <utility>
#include <vector>
#include <version>

#if defined(__cpp_lib_forward_like)
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
  static_assert(yk::to_array_of<double>(3.0, 1.0, 4.0, 1.0, 5.0) == std::array{3.0, 1.0, 4.0, 1.0, 5.0});
}

BOOST_AUTO_TEST_SUITE_END()  // yk_util
