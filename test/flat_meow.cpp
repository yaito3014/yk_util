#include <yk/flat_set.hpp>

#include <boost/test/unit_test.hpp>

#include <vector>

BOOST_AUTO_TEST_SUITE(flat_meow)

BOOST_AUTO_TEST_CASE(flat_set)
{
  //
  std::vector vec{3, 1, 4, 1, 5};
  yk::flat_set<int> fs(std::move(vec));
}

BOOST_AUTO_TEST_SUITE_END()