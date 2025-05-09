#include "yk/compare.hpp"

#include <boost/test/unit_test.hpp>

#include <compare>
#include <print>
#include <ranges>
#include <string>
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
    const auto comp = then(then(extract(&S::id), extract(&S::name)), extract(&S::height));

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
    struct name_extractor {
      std::string& log;

      name_extractor(std::string& s) : log(s) { log += "NC"; }
      name_extractor(const name_extractor& other) noexcept : log(other.log) { log += "CC"; }
      name_extractor(name_extractor&& other) noexcept : log(other.log) { log += "MC"; }
      ~name_extractor() { log += "D"; }
      name_extractor& operator=(const name_extractor&) noexcept
      {
        log += "CA";
        return *this;
      }
      name_extractor& operator=(name_extractor&&) noexcept
      {
        log += "MA";
        return *this;
      }
      const std::string& operator()(const S& s) const { return s.name; }
    };

    {
      std::string log;
      {
        const yk::compare::comparator auto comp = extract(&S::id) | name_extractor{log};
        // construction -> NC
        // move_construction in extract_comparator -> MC
        // move_construction in then_comparator -> MC
      }
      std::println("{}", log);
      // BOOST_TEST(log == "NCMCMCDDD");
    }
  }

  // composing range adaptor closure
  // {
  //   const auto range_adaptor_closure = std::views::reverse | std::ranges::to<std::string>();
  //   const auto comp = extract(&S::name) | range_adaptor_closure;
  //   BOOST_TEST((comp(S{1, "foo", 3.14}, S{2, "bar", 1.41}) < 0));
  // }

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

namespace {

[[maybe_unused]] std::strong_ordering compare(int x, int y) { return x <=> y; }

}  // namespace

BOOST_AUTO_TEST_CASE(ClosureTypeTraits)
{
  {
    auto comp [[maybe_unused]] = yk::compare::then_comparator{compare, compare};
    using F = std::strong_ordering(int, int);
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<F&>,
      yk::compare::wrapper_comparator<F&>
    >>);
  }

  {
    constexpr struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    } function_object;
    auto comp = yk::compare::then_comparator{function_object, function_object};
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<const fn&>,
      yk::compare::wrapper_comparator<const fn&>
    >>);
  }
  
  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    } function_object;
    auto comp = yk::compare::then_comparator{function_object, function_object};
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<fn&>,
      yk::compare::wrapper_comparator<fn&>
    >>);
  }

  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    };
    auto comp = yk::compare::then_comparator{ fn{}, fn{} };
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<fn>,
      yk::compare::wrapper_comparator<fn>
    >>);
  }
  ///////////////////////////////////////////////////////////////////////
  {
    auto comp [[maybe_unused]] = yk::comparators::then(compare, compare);
    using F = std::strong_ordering(int, int);
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<F&>,
      yk::compare::wrapper_comparator<F&>
    >>);
  }

  {
    constexpr struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    } function_object;
    auto comp = yk::comparators::then(function_object, function_object);
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<const fn&>,
      yk::compare::wrapper_comparator<const fn&>
    >>);
  }
  
  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    } function_object;
    auto comp = yk::comparators::then(function_object, function_object);
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<fn&>,
      yk::compare::wrapper_comparator<fn&>
    >>);
  }

  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    };
    auto comp = yk::comparators::then(fn{}, fn{});
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<fn>,
      yk::compare::wrapper_comparator<fn>
    >>);
  }

  {
    auto closure [[maybe_unused]] = yk::comparators::then(&compare);
    using F = std::strong_ordering(int, int);
    static_assert(std::is_same_v<decltype(closure), yk::compare::detail::comp_then_closure<F*>>);

    auto comp = yk::comparators::wrap(&compare) | closure;
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<const F*>,
      yk::compare::wrapper_comparator<const F*&>
    >>);
  }

  {
    auto closure [[maybe_unused]] = yk::comparators::then(compare);
    using F = std::strong_ordering(int, int);
    static_assert(std::is_same_v<decltype(closure), yk::compare::detail::comp_then_closure<F&>>);

    auto comp = yk::comparators::wrap(compare) | closure;
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<F&>,
      yk::compare::wrapper_comparator<F&>
    >>);
  }
  
  {
    constexpr struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    } function_object;
    auto closure [[maybe_unused]] = yk::comparators::then(function_object);
    static_assert(std::is_same_v<decltype(closure), yk::compare::detail::comp_then_closure<const fn&>>);

    auto comp = function_object | closure;
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<const fn&>,
      yk::compare::wrapper_comparator<const fn&>
    >>);
  }
  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    } function_object;
    auto closure [[maybe_unused]] = yk::comparators::then(function_object);
    static_assert(std::is_same_v<decltype(closure), yk::compare::detail::comp_then_closure<fn&>>);

    auto comp = function_object | closure;
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<fn&>,
      yk::compare::wrapper_comparator<fn&>
    >>);
  }
  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    } function_object;
    auto comp = function_object | yk::comparators::then(function_object);
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<fn&>,
      yk::compare::wrapper_comparator<fn&>
    >>);
  }
  {
    struct A {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    };
    struct B {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    };
    auto closure [[maybe_unused]] = yk::comparators::then(A{});
    static_assert(std::is_same_v<decltype(closure), yk::compare::detail::comp_then_closure<A>>);

    auto comp = B{} |  closure;
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<B>,
      yk::compare::wrapper_comparator<A&>
    >>);
  }
  {
    struct A {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    };
    struct B {
      std::strong_ordering operator()(int x, int y) const noexcept {
        return x <=> y;
      }
    };
    auto comp = B{} | yk::comparators::then(A{});
    static_assert(std::is_same_v<decltype(comp), yk::compare::then_comparator<
      yk::compare::wrapper_comparator<B>,
      yk::compare::wrapper_comparator<A>
    >>);
  }
}

BOOST_AUTO_TEST_SUITE_END()
