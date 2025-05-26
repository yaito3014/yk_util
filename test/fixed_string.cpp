#include <yk/fixed_string.hpp>

#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <string_view>

BOOST_AUTO_TEST_SUITE(fixed_string)

BOOST_AUTO_TEST_CASE(fixed_string)
{
  using namespace std::string_view_literals;
  constexpr yk::fixed_string str = "foobar";
  BOOST_TEST((std::ranges::equal(str, "foobar"sv)));
}

BOOST_AUTO_TEST_SUITE_END()
