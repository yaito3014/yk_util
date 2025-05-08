#include "yk/compare/comparator.hpp"
#include "yk/compare/then.hpp"

#include <boost/test/unit_test.hpp>

#include <compare>

BOOST_AUTO_TEST_SUITE(Compare)

struct my_data {
  int id;
  std::string name;

  friend constexpr auto operator<=>(const my_data& x, const my_data& y) noexcept
  {
    return (x.id <=> y.id) | yk::compare::then_with([&] { return x.name <=> y.name; });
  }
};

BOOST_AUTO_TEST_CASE(then)
{
  BOOST_TEST((yk::compare::then(1 <=> 2, 3 <=> 4) < 0));
  BOOST_TEST((yk::compare::then(1 <=> 1, 3 <=> 4) < 0));
  BOOST_TEST((yk::compare::then(2 <=> 1, 3 <=> 4) > 0));

  BOOST_TEST((yk::compare::then(1 <=> 2, 4 <=> 3) < 0));
  BOOST_TEST((yk::compare::then(1 <=> 1, 4 <=> 3) > 0));
  BOOST_TEST((yk::compare::then(2 <=> 1, 4 <=> 3) > 0));

  BOOST_TEST((yk::compare::then(1 <=> 2, 3 <=> 3) < 0));
  BOOST_TEST((yk::compare::then(1 <=> 1, 3 <=> 3) == 0));
  BOOST_TEST((yk::compare::then(2 <=> 1, 3 <=> 3) > 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then(3 <=> 4)) < 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then(3 <=> 4)) < 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then(3 <=> 4)) > 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then(4 <=> 3)) < 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then(4 <=> 3)) > 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then(4 <=> 3)) > 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then(3 <=> 3)) < 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then(3 <=> 3)) == 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then(3 <=> 3)) > 0));

  BOOST_TEST((yk::compare::then_with(1 <=> 2, [] { return 3 <=> 4; }) < 0));
  BOOST_TEST((yk::compare::then_with(1 <=> 1, [] { return 3 <=> 4; }) < 0));
  BOOST_TEST((yk::compare::then_with(2 <=> 1, [] { return 3 <=> 4; }) > 0));

  BOOST_TEST((yk::compare::then_with(1 <=> 2, [] { return 4 <=> 3; }) < 0));
  BOOST_TEST((yk::compare::then_with(1 <=> 1, [] { return 4 <=> 3; }) > 0));
  BOOST_TEST((yk::compare::then_with(2 <=> 1, [] { return 4 <=> 3; }) > 0));

  BOOST_TEST((yk::compare::then_with(1 <=> 2, [] { return 3 <=> 3; }) < 0));
  BOOST_TEST((yk::compare::then_with(1 <=> 1, [] { return 3 <=> 3; }) == 0));
  BOOST_TEST((yk::compare::then_with(2 <=> 1, [] { return 3 <=> 3; }) > 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then_with([] { return 3 <=> 4; })) < 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then_with([] { return 3 <=> 4; })) < 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then_with([] { return 3 <=> 4; })) > 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then_with([] { return 4 <=> 3; })) < 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then_with([] { return 4 <=> 3; })) > 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then_with([] { return 4 <=> 3; })) > 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then_with([] { return 3 <=> 3; })) < 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then_with([] { return 3 <=> 3; })) == 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then_with([] { return 3 <=> 3; })) > 0));
}

BOOST_AUTO_TEST_SUITE_END()
