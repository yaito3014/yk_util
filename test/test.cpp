#include "yk/util/forward_like.hpp"
#include "yk/util/pack_indexing.hpp"
#include "yk/util/specialization_of.hpp"

#define BOOST_TEST_MODULE yk_util_test
#include <boost/test/unit_test.hpp>

#include <string>
#include <vector>

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
  // clang-format on
}

BOOST_AUTO_TEST_SUITE_END()  // yk_util
