#include "yk/compare.hpp"

#include <boost/test/unit_test.hpp>

#include <compare>
#include <limits>
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
        BOOST_TEST(log == "NCMCMCDD");
        BOOST_TEST((comp(S{0, "foo", 3.14}, S{1, "bar", 6.28}) < 0));
      }
      BOOST_TEST(log == "NCMCMCDDD");
    }
  }

  // composing range adaptor closure
  {
    const auto range_adaptor_closure = std::views::reverse | std::ranges::to<std::string>();
    const auto comp = extract(&S::name) | range_adaptor_closure;
    BOOST_TEST((comp(S{1, "foo", 3.14}, S{2, "bar", 1.41}) < 0));
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

BOOST_AUTO_TEST_CASE(NonCopyable)
{
  struct int_comparator {
    int_comparator() = default;
    int_comparator(const int_comparator&) = delete;
    int_comparator(int_comparator&&) = default;
    std::strong_ordering operator()(int a, int b) const noexcept { return a <=> b; }
  };

  auto comp [[maybe_unused]] = int_comparator{} | yk::comparators::then(int_comparator{});
  auto closure [[maybe_unused]] = yk::comparators::then(int_comparator{}) | yk::comparators::then(int_comparator{});
}

namespace {

[[maybe_unused]] std::strong_ordering compare(int x, int y) { return x <=> y; }

}  // namespace

BOOST_AUTO_TEST_CASE(ClosureTypeTraits)
{
  {
    auto comp [[maybe_unused]] = yk::compare::then_comparator{compare, compare};
    using F = std::strong_ordering(int, int);
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<F&>, yk::compare::wrapper_comparator<F&>>>
    );
  }

  {
    constexpr struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    } function_object;
    auto comp = yk::compare::then_comparator{function_object, function_object};
    static_assert(
        std::is_same_v<
            decltype(comp), yk::compare::then_comparator<
                                yk::compare::wrapper_comparator<const fn&>, yk::compare::wrapper_comparator<const fn&>>>
    );
  }

  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    } function_object;
    auto comp = yk::compare::then_comparator{function_object, function_object};
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<fn&>, yk::compare::wrapper_comparator<fn&>>>
    );
  }

  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    };
    auto comp = yk::compare::then_comparator{fn{}, fn{}};
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<fn>, yk::compare::wrapper_comparator<fn>>>
    );
  }
  ///////////////////////////////////////////////////////////////////////
  {
    auto comp [[maybe_unused]] = yk::comparators::then(compare, compare);
    using F = std::strong_ordering(int, int);
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<F&>, yk::compare::wrapper_comparator<F&>>>
    );
  }

  {
    constexpr struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    } function_object;
    auto comp = yk::comparators::then(function_object, function_object);
    static_assert(
        std::is_same_v<
            decltype(comp), yk::compare::then_comparator<
                                yk::compare::wrapper_comparator<const fn&>, yk::compare::wrapper_comparator<const fn&>>>
    );
  }

  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    } function_object;
    auto comp = yk::comparators::then(function_object, function_object);
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<fn&>, yk::compare::wrapper_comparator<fn&>>>
    );
  }

  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    };
    auto comp = yk::comparators::then(fn{}, fn{});
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<fn>, yk::compare::wrapper_comparator<fn>>>
    );
  }

  {
    auto closure [[maybe_unused]] = yk::comparators::then(&compare);
    using F = std::strong_ordering(int, int);
    static_assert(std::is_same_v<decltype(closure), yk::compare::detail::comp_then_closure<F*>>);
    static_assert(!yk::compare::detail::RangeAdaptorClosure<decltype(closure)>);
    auto comp = yk::comparators::wrap(&compare) | closure;
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<F*>, yk::compare::wrapper_comparator<F*&>>>
    );
  }

  {
    auto closure [[maybe_unused]] = yk::comparators::then(compare);
    using F = std::strong_ordering(int, int);
    static_assert(std::is_same_v<decltype(closure), yk::compare::detail::comp_then_closure<F&>>);
    static_assert(!yk::compare::detail::RangeAdaptorClosure<decltype(closure)>);
    auto comp = yk::comparators::wrap(compare) | closure;
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<F&>, yk::compare::wrapper_comparator<F&>>>
    );
  }

  {
    constexpr struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    } function_object;
    auto closure [[maybe_unused]] = yk::comparators::then(function_object);
    static_assert(std::is_same_v<decltype(closure), yk::compare::detail::comp_then_closure<const fn&>>);
    static_assert(!yk::compare::detail::RangeAdaptorClosure<decltype(closure)>);
    auto comp = function_object | closure;
    static_assert(
        std::is_same_v<
            decltype(comp), yk::compare::then_comparator<
                                yk::compare::wrapper_comparator<const fn&>, yk::compare::wrapper_comparator<const fn&>>>
    );
  }
  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    } function_object;
    auto closure [[maybe_unused]] = yk::comparators::then(function_object);
    static_assert(std::is_same_v<decltype(closure), yk::compare::detail::comp_then_closure<fn&>>);
    static_assert(!yk::compare::detail::RangeAdaptorClosure<decltype(closure)>);
    auto comp = function_object | closure;
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<fn&>, yk::compare::wrapper_comparator<fn&>>>
    );
  }
  {
    struct fn {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    } function_object;
    auto comp = function_object | yk::comparators::then(function_object);
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<fn&>, yk::compare::wrapper_comparator<fn&>>>
    );
  }
  {
    struct A {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    };
    struct B {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    };
    auto closure [[maybe_unused]] = yk::comparators::then(A{});
    static_assert(std::is_same_v<decltype(closure), yk::compare::detail::comp_then_closure<A>>);
    static_assert(!yk::compare::detail::RangeAdaptorClosure<decltype(closure)>);
    auto comp = B{} | closure;
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<B>, yk::compare::wrapper_comparator<A&>>>
    );
  }
  {
    struct A {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    };
    struct B {
      std::strong_ordering operator()(int x, int y) const noexcept { return x <=> y; }
    };
    auto comp = B{} | yk::comparators::then(A{});
    static_assert(
        std::is_same_v<
            decltype(comp),
            yk::compare::then_comparator<yk::compare::wrapper_comparator<B>, yk::compare::wrapper_comparator<A>>>
    );
  }
}

BOOST_AUTO_TEST_CASE(promotion)
{
  using namespace yk::comparators;

  // partial | promote(partial)
  {
    const auto partial_comparator = [](double a, double b) -> std::partial_ordering { return a <=> b; };

    auto comp = partial_comparator | promote(partial_comparator);
    BOOST_TEST((comp(3.14, 1.41) == std::weak_ordering::greater));
    BOOST_TEST((comp(3.14, std::numeric_limits<double>::quiet_NaN()) == std::partial_ordering::unordered));
  }

  // partial | promote(weak)
  {
    const auto partial_comparator = [](double a, double b) -> std::partial_ordering { return a <=> b; };
    const auto weak_comparator = [](double a, double b) -> std::weak_ordering { return std::weak_order(a, b); };

    auto comp = partial_comparator | promote(weak_comparator);
    BOOST_TEST((comp(3.14, 1.41) == std::weak_ordering::greater));
    BOOST_TEST((comp(3.14, std::numeric_limits<double>::quiet_NaN()) == std::weak_ordering::less));
  }

  // partial | promote(strong)
  {
    const auto partial_comparator = [](double a, double b) -> std::partial_ordering { return a <=> b; };
    const auto strong_comparator = [](double a, double b) -> std::strong_ordering { return std::strong_order(a, b); };

    auto comp = partial_comparator | promote(strong_comparator);
    BOOST_TEST((comp(3.14, 1.41) == std::strong_ordering::greater));
    BOOST_TEST((comp(3.14, std::numeric_limits<double>::quiet_NaN()) == std::strong_ordering::less));
  }

  // weak | promote(weak)
  {
    const auto weak_comparator = [](const std::string& a, const std::string& b) -> std::weak_ordering {
      return a.size() <=> b.size();
    };
    auto comp = weak_comparator | promote(weak_comparator);
    BOOST_TEST((comp("fooo", "bar") == std::weak_ordering::greater));
    BOOST_TEST((comp("foo", "bar") == std::weak_ordering::equivalent));
  }

  // weak | promote(strong)
  {
    const auto weak_comparator = [](const std::string& a, const std::string& b) -> std::weak_ordering {
      return a.size() <=> b.size();
    };
    const auto strong_comparator = [](const std::string& a, const std::string& b) -> std::strong_ordering {
      return a <=> b;
    };
    auto comp = weak_comparator | promote(strong_comparator);
    BOOST_TEST((comp("fooo", "bar") == std::strong_ordering::greater));
    BOOST_TEST((comp("foo", "bar") == std::strong_ordering::greater));
  }

  // strong | promote(strong)
  // unsupported; use then() instead.
}

BOOST_AUTO_TEST_SUITE_END()
