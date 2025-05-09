#include "yk/compare/comparator.hpp"
#include "yk/compare/then.hpp"

#include <boost/test/unit_test.hpp>

#include <compare>

BOOST_AUTO_TEST_SUITE(Compare)

BOOST_AUTO_TEST_CASE(compare_then)
{
  using namespace yk::compare;

  BOOST_TEST((then(1 <=> 2, 3 <=> 4) < 0));
  BOOST_TEST((then(2 <=> 1, 3 <=> 4) > 0));
  BOOST_TEST((then(1 <=> 1, 3 <=> 4) < 0));

  BOOST_TEST((then(1 <=> 2, 4 <=> 3) < 0));
  BOOST_TEST((then(2 <=> 1, 4 <=> 3) > 0));
  BOOST_TEST((then(1 <=> 1, 4 <=> 3) > 0));

  BOOST_TEST((then(1 <=> 2, 3 <=> 3) < 0));
  BOOST_TEST((then(2 <=> 1, 3 <=> 3) > 0));
  BOOST_TEST((then(1 <=> 1, 3 <=> 3) == 0));

  BOOST_TEST((((1 <=> 2) | then(3 <=> 4)) < 0));
  BOOST_TEST((((2 <=> 1) | then(3 <=> 4)) > 0));
  BOOST_TEST((((1 <=> 1) | then(3 <=> 4)) < 0));

  BOOST_TEST((((1 <=> 2) | then(4 <=> 3)) < 0));
  BOOST_TEST((((2 <=> 1) | then(4 <=> 3)) > 0));
  BOOST_TEST((((1 <=> 1) | then(4 <=> 3)) > 0));

  BOOST_TEST((((1 <=> 2) | then(3 <=> 3)) < 0));
  BOOST_TEST((((2 <=> 1) | then(3 <=> 3)) > 0));
  BOOST_TEST((((1 <=> 1) | then(3 <=> 3)) == 0));

  BOOST_TEST((then_with(1 <=> 2, [] { return 3 <=> 4; }) < 0));
  BOOST_TEST((then_with(2 <=> 1, [] { return 3 <=> 4; }) > 0));
  BOOST_TEST((then_with(1 <=> 1, [] { return 3 <=> 4; }) < 0));

  BOOST_TEST((then_with(1 <=> 2, [] { return 4 <=> 3; }) < 0));
  BOOST_TEST((then_with(2 <=> 1, [] { return 4 <=> 3; }) > 0));
  BOOST_TEST((then_with(1 <=> 1, [] { return 4 <=> 3; }) > 0));

  BOOST_TEST((then_with(1 <=> 2, [] { return 3 <=> 3; }) < 0));
  BOOST_TEST((then_with(2 <=> 1, [] { return 3 <=> 3; }) > 0));
  BOOST_TEST((then_with(1 <=> 1, [] { return 3 <=> 3; }) == 0));

  BOOST_TEST((((1 <=> 2) | then_with([] { return 3 <=> 4; })) < 0));
  BOOST_TEST((((2 <=> 1) | then_with([] { return 3 <=> 4; })) > 0));
  BOOST_TEST((((1 <=> 1) | then_with([] { return 3 <=> 4; })) < 0));

  BOOST_TEST((((1 <=> 2) | then_with([] { return 4 <=> 3; })) < 0));
  BOOST_TEST((((2 <=> 1) | then_with([] { return 4 <=> 3; })) > 0));
  BOOST_TEST((((1 <=> 1) | then_with([] { return 4 <=> 3; })) > 0));

  BOOST_TEST((((1 <=> 2) | then_with([] { return 3 <=> 3; })) < 0));
  BOOST_TEST((((2 <=> 1) | then_with([] { return 3 <=> 3; })) > 0));
  BOOST_TEST((((1 <=> 1) | then_with([] { return 3 <=> 3; })) == 0));
}

BOOST_AUTO_TEST_CASE(extract_and_comparator_then)
{
  using namespace yk::comparators;

  {
    struct S {
      int id;
      std::string name;
    };
    const auto comp = extract(&S::id) | then(extract(&S::name));

    BOOST_TEST((comp(S{1, "foo"}, S{2, "bar"}) < 0));
    BOOST_TEST((comp(S{2, "foo"}, S{1, "bar"}) > 0));
    BOOST_TEST((comp(S{1, "foo"}, S{1, "bar"}) > 0));

    BOOST_TEST((comp(S{1, "bar"}, S{2, "foo"}) < 0));
    BOOST_TEST((comp(S{2, "bar"}, S{1, "foo"}) > 0));
    BOOST_TEST((comp(S{1, "bar"}, S{1, "foo"}) < 0));

    BOOST_TEST((comp(S{1, "foo"}, S{2, "foo"}) < 0));
    BOOST_TEST((comp(S{2, "foo"}, S{1, "foo"}) > 0));
    BOOST_TEST((comp(S{1, "foo"}, S{1, "foo"}) == 0));
  }

  // short-hand syntax
  {
    struct S {
      int id;
      std::string name;
      double height;
    };
    const yk::compare::comparator auto comp1 = extract(&S::id);
    const yk::compare::comparator auto comp2 = comp1 | &S::name;
    const yk::compare::comparator auto comp3 = comp2 | &S::height;

    BOOST_TEST((comp3(S{1, "foo", 3.14}, S{2, "bar", 3.14}) < 0));
    BOOST_TEST((comp3(S{2, "foo", 3.14}, S{1, "bar", 3.14}) > 0));
    BOOST_TEST((comp3(S{1, "foo", 3.14}, S{1, "bar", 3.14}) > 0));

    BOOST_TEST((comp3(S{1, "bar", 3.14}, S{2, "foo", 3.14}) < 0));
    BOOST_TEST((comp3(S{2, "bar", 3.14}, S{1, "foo", 3.14}) > 0));
    BOOST_TEST((comp3(S{1, "bar", 3.14}, S{1, "foo", 3.14}) < 0));

    BOOST_TEST((comp3(S{1, "foo", 3.14}, S{2, "foo", 3.14}) < 0));
    BOOST_TEST((comp3(S{2, "foo", 3.14}, S{1, "foo", 3.14}) > 0));
    BOOST_TEST((comp3(S{1, "foo", 3.14}, S{1, "foo", 3.14}) == 0));
  }
}

BOOST_AUTO_TEST_SUITE_END()
