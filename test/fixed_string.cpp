#include <yk/fixed_string.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(fixed_string)

BOOST_AUTO_TEST_CASE(fixed_string)
{
  using namespace yk::fixed_string_literals;

  constexpr auto str [[maybe_unused]] = "foo"_fixed;
}

BOOST_AUTO_TEST_SUITE_END()
