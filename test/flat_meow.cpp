#include <yk/flat_set.hpp>

#include <boost/test/unit_test.hpp>

#include <vector>

BOOST_AUTO_TEST_SUITE(flat_meow)

BOOST_AUTO_TEST_CASE(flat_set)
{
  {
    auto vec = yk::flat_set<int>{std::vector{3, 1, 4, 1, 5}}.extract();
    BOOST_ASSERT(std::ranges::is_sorted(vec));
  }
  {
    yk::flat_set<int> fs;
    fs.insert({3, 1, 4, 1, 5});
    BOOST_TEST(std::ranges::equal(fs, std::vector{1, 3, 4, 5}));
  }
  {
    yk::flat_set<int> fs{3, 1, 4, 1, 5};
    fs.insert({9, 2, 6, 5, 3, 5});
    BOOST_TEST(std::ranges::equal(fs, std::vector{1, 2, 3, 4, 5, 6, 9}));
  }
}

BOOST_AUTO_TEST_SUITE_END()
