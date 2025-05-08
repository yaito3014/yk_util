#include "yk/compare/comparator.hpp"
#include "yk/compare/then.hpp"

#include <boost/test/unit_test.hpp>

#include <compare>

BOOST_AUTO_TEST_SUITE(Compare)

BOOST_AUTO_TEST_CASE(compare_then)
{
  BOOST_TEST((yk::compare::then(1 <=> 2, 3 <=> 4) < 0));
  BOOST_TEST((yk::compare::then(2 <=> 1, 3 <=> 4) > 0));
  BOOST_TEST((yk::compare::then(1 <=> 1, 3 <=> 4) < 0));

  BOOST_TEST((yk::compare::then(1 <=> 2, 4 <=> 3) < 0));
  BOOST_TEST((yk::compare::then(2 <=> 1, 4 <=> 3) > 0));
  BOOST_TEST((yk::compare::then(1 <=> 1, 4 <=> 3) > 0));

  BOOST_TEST((yk::compare::then(1 <=> 2, 3 <=> 3) < 0));
  BOOST_TEST((yk::compare::then(2 <=> 1, 3 <=> 3) > 0));
  BOOST_TEST((yk::compare::then(1 <=> 1, 3 <=> 3) == 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then(3 <=> 4)) < 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then(3 <=> 4)) > 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then(3 <=> 4)) < 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then(4 <=> 3)) < 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then(4 <=> 3)) > 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then(4 <=> 3)) > 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then(3 <=> 3)) < 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then(3 <=> 3)) > 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then(3 <=> 3)) == 0));

  BOOST_TEST((yk::compare::then_with(1 <=> 2, [] { return 3 <=> 4; }) < 0));
  BOOST_TEST((yk::compare::then_with(2 <=> 1, [] { return 3 <=> 4; }) > 0));
  BOOST_TEST((yk::compare::then_with(1 <=> 1, [] { return 3 <=> 4; }) < 0));

  BOOST_TEST((yk::compare::then_with(1 <=> 2, [] { return 4 <=> 3; }) < 0));
  BOOST_TEST((yk::compare::then_with(2 <=> 1, [] { return 4 <=> 3; }) > 0));
  BOOST_TEST((yk::compare::then_with(1 <=> 1, [] { return 4 <=> 3; }) > 0));

  BOOST_TEST((yk::compare::then_with(1 <=> 2, [] { return 3 <=> 3; }) < 0));
  BOOST_TEST((yk::compare::then_with(2 <=> 1, [] { return 3 <=> 3; }) > 0));
  BOOST_TEST((yk::compare::then_with(1 <=> 1, [] { return 3 <=> 3; }) == 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then_with([] { return 3 <=> 4; })) < 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then_with([] { return 3 <=> 4; })) > 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then_with([] { return 3 <=> 4; })) < 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then_with([] { return 4 <=> 3; })) < 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then_with([] { return 4 <=> 3; })) > 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then_with([] { return 4 <=> 3; })) > 0));

  BOOST_TEST((((1 <=> 2) | yk::compare::then_with([] { return 3 <=> 3; })) < 0));
  BOOST_TEST((((2 <=> 1) | yk::compare::then_with([] { return 3 <=> 3; })) > 0));
  BOOST_TEST((((1 <=> 1) | yk::compare::then_with([] { return 3 <=> 3; })) == 0));
}

BOOST_AUTO_TEST_CASE(comparator_then)
{
  using namespace yk::comparators;

  struct my_data {
    int id;
    std::string name;
  };

  const auto comp = extract{&my_data::id} | then(extract{&my_data::name});

  static_assert(yk::compare::comparator<decltype(comp), my_data, my_data>);

  BOOST_TEST((comp(my_data{1, "foo"}, my_data{2, "bar"}) < 0));
  BOOST_TEST((comp(my_data{2, "foo"}, my_data{1, "bar"}) > 0));
  BOOST_TEST((comp(my_data{1, "foo"}, my_data{1, "bar"}) > 0));

  BOOST_TEST((comp(my_data{1, "bar"}, my_data{2, "foo"}) < 0));
  BOOST_TEST((comp(my_data{2, "bar"}, my_data{1, "foo"}) > 0));
  BOOST_TEST((comp(my_data{1, "bar"}, my_data{1, "foo"}) < 0));

  BOOST_TEST((comp(my_data{1, "foo"}, my_data{2, "foo"}) < 0));
  BOOST_TEST((comp(my_data{2, "foo"}, my_data{1, "foo"}) > 0));
  BOOST_TEST((comp(my_data{1, "foo"}, my_data{1, "foo"}) == 0));
}

BOOST_AUTO_TEST_SUITE_END()
