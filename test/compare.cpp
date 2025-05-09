#include "yk/compare.hpp"

#include <boost/test/unit_test.hpp>

#include <compare>
#include <type_traits>

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

  struct S {
    int id;
    std::string name;
    double height;
  };

  // using `then` comparator adaptor
  {
    const auto comp = then(then(extract(&S::id), extract(&S::id)), extract(&S::height));

    BOOST_TEST((comp(S{1, "foo", 3.14}, S{2, "bar", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "foo", 3.14}, S{1, "bar", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "foo", 3.14}, S{1, "bar", 3.14}) > 0));

    BOOST_TEST((comp(S{1, "bar", 3.14}, S{2, "foo", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "bar", 3.14}, S{1, "foo", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "bar", 3.14}, S{1, "foo", 3.14}) < 0));

    BOOST_TEST((comp(S{1, "foo", 3.14}, S{2, "foo", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "foo", 3.14}, S{1, "foo", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "foo", 3.14}, S{1, "foo", 3.14}) == 0));
  }

  // comparator adaptor is pipeable
  {
    const auto comp = extract(&S::id) | then(extract(&S::name)) | then(extract(&S::height));

    BOOST_TEST((comp(S{1, "foo", 3.14}, S{2, "bar", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "foo", 3.14}, S{1, "bar", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "foo", 3.14}, S{1, "bar", 3.14}) > 0));

    BOOST_TEST((comp(S{1, "bar", 3.14}, S{2, "foo", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "bar", 3.14}, S{1, "foo", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "bar", 3.14}, S{1, "foo", 3.14}) < 0));

    BOOST_TEST((comp(S{1, "foo", 3.14}, S{2, "foo", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "foo", 3.14}, S{1, "foo", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "foo", 3.14}, S{1, "foo", 3.14}) == 0));
  }

  // comparator adaptor closure is associative
  {
    const auto comp = extract(&S::id) | (then(extract(&S::name)) | then(extract(&S::height)));

    BOOST_TEST((comp(S{1, "foo", 3.14}, S{2, "bar", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "foo", 3.14}, S{1, "bar", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "foo", 3.14}, S{1, "bar", 3.14}) > 0));

    BOOST_TEST((comp(S{1, "bar", 3.14}, S{2, "foo", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "bar", 3.14}, S{1, "foo", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "bar", 3.14}, S{1, "foo", 3.14}) < 0));

    BOOST_TEST((comp(S{1, "foo", 3.14}, S{2, "foo", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "foo", 3.14}, S{1, "foo", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "foo", 3.14}, S{1, "foo", 3.14}) == 0));
  }

  // short-hand syntax
  {
    const yk::compare::comparator auto comp = extract(&S::id) | &S::name | &S::height;

    BOOST_TEST((comp(S{1, "foo", 3.14}, S{2, "bar", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "foo", 3.14}, S{1, "bar", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "foo", 3.14}, S{1, "bar", 3.14}) > 0));

    BOOST_TEST((comp(S{1, "bar", 3.14}, S{2, "foo", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "bar", 3.14}, S{1, "foo", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "bar", 3.14}, S{1, "foo", 3.14}) < 0));

    BOOST_TEST((comp(S{1, "foo", 3.14}, S{2, "foo", 3.14}) < 0));
    BOOST_TEST((comp(S{2, "foo", 3.14}, S{1, "foo", 3.14}) > 0));
    BOOST_TEST((comp(S{1, "foo", 3.14}, S{1, "foo", 3.14}) == 0));
  }

  {
    const auto comp = std::strong_order | then(std::strong_order);
    static_assert(std::is_same_v<std::invoke_result_t<decltype(comp), int, int>, std::strong_ordering>);
  }
  {
    const auto comp = std::weak_order | then(std::weak_order);
    static_assert(std::is_same_v<std::invoke_result_t<decltype(comp), int, int>, std::weak_ordering>);
  }
  {
    const auto comp = std::partial_order | then(std::partial_order);
    static_assert(std::is_same_v<std::invoke_result_t<decltype(comp), int, int>, std::partial_ordering>);
  }

  {
    const auto comp = std::strong_order | then(std::weak_order);
    static_assert(std::is_same_v<std::invoke_result_t<decltype(comp), int, int>, std::weak_ordering>);
  }
  {
    const auto comp = std::weak_order | then(std::strong_order);
    static_assert(std::is_same_v<std::invoke_result_t<decltype(comp), int, int>, std::weak_ordering>);
  }
  {
    const auto comp = std::weak_order | then(std::partial_order);
    static_assert(std::is_same_v<std::invoke_result_t<decltype(comp), int, int>, std::partial_ordering>);
  }
  {
    const auto comp = std::partial_order | then(std::weak_order);
    static_assert(std::is_same_v<std::invoke_result_t<decltype(comp), int, int>, std::partial_ordering>);
  }
  {
    const auto comp = std::strong_order | then(std::partial_order);
    static_assert(std::is_same_v<std::invoke_result_t<decltype(comp), int, int>, std::partial_ordering>);
  }
  {
    const auto comp = std::partial_order | then(std::strong_order);
    static_assert(std::is_same_v<std::invoke_result_t<decltype(comp), int, int>, std::partial_ordering>);
  }
}

BOOST_AUTO_TEST_SUITE_END()
