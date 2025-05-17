#include <yk/flat_set.hpp>

#include <boost/test/unit_test.hpp>

#include <vector>

BOOST_AUTO_TEST_SUITE(flat_meow)

BOOST_AUTO_TEST_CASE(flat_set)
{
  auto vec = yk::flat_set<int>{std::vector{3, 1, 4, 1, 5}}.extract();
  BOOST_ASSERT(std::ranges::is_sorted(vec));
}

BOOST_AUTO_TEST_SUITE_END()
