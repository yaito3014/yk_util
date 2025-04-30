#include "yk/allocator/default_init_allocator.hpp"
#include "yk/ranges/concat.hpp"
#include "yk/printt.hpp"
#include "yk/stack.hpp"
#include "yk/throwt.hpp"

#include "test_utility.hpp"

#define BOOST_TEST_MODULE yk_util_test
#if YK_BUILD_UNIT_TEST_FRAMEWORK
#include <boost/test/included/unit_test.hpp>
#else
#include <boost/test/unit_test.hpp>
#endif

#include <algorithm>
#include <compare>
#include <forward_list>
#include <functional>
#include <iterator>
#include <list>
#include <memory>
#include <ranges>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>
#include <version>

#include <cstdint>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(yk_util)

BOOST_AUTO_TEST_CASE(Allocator) {
  {
    unsigned char uninitialized_storage[sizeof(std::uint32_t)] = {0xde, 0xad, 0xbe, 0xef};
    alignas(std::uint32_t) unsigned char storage[sizeof(std::uint32_t)];
    std::ranges::copy(uninitialized_storage, storage);

    std::allocator<std::uint32_t> alloc;
    std::allocator_traits<std::allocator<std::uint32_t>>::construct(alloc, reinterpret_cast<std::uint32_t*>(storage));

    BOOST_TEST(!std::ranges::equal(uninitialized_storage, storage));
    BOOST_TEST(std::ranges::equal(std::vector<unsigned char>{0, 0, 0, 0}, storage));
  }
  {
    unsigned char uninitialized_storage[sizeof(std::uint32_t)] = {0xde, 0xad, 0xbe, 0xef};
    alignas(std::uint32_t) unsigned char storage[sizeof(std::uint32_t)];
    std::ranges::copy(uninitialized_storage, storage);

    yk::default_init_allocator<std::uint32_t> alloc;
    alloc.construct(storage);

    BOOST_TEST(std::ranges::equal(uninitialized_storage, storage));
  }
}

BOOST_AUTO_TEST_CASE(Stack) {
  yk::stack<int> s{3, 1, 4, 1, 5};

  s.shrink_to_fit();
  BOOST_TEST(s.capacity() == 5);
  s.reserve(10);
  BOOST_TEST(s.capacity() >= 10);
  BOOST_TEST(s.capacity() <= s.max_size());

  BOOST_TEST(!s.empty());
  s.clear();
  BOOST_TEST(s.empty());
}

BOOST_AUTO_TEST_CASE(Concat) {
  using namespace std::literals;
  // non-const test
  {
    auto rng = yk::views::concat("foo"sv, "bar"sv, "baz"sv);
    static_assert(std::ranges::random_access_range<decltype(rng)>);
    static_assert(std::ranges::view<decltype(rng)>);
    std::random_access_iterator auto iter = std::ranges::begin(rng);
    std::random_access_iterator auto sent = std::ranges::end(rng);
    BOOST_TEST((iter != sent));
    BOOST_TEST((sent - iter) == 9);
    BOOST_TEST((iter == iter));
    BOOST_TEST((iter - iter) == 0);
    BOOST_TEST((std::default_sentinel - iter) == 9);
    BOOST_TEST((iter - std::default_sentinel) == -9);
    BOOST_TEST(*(iter + 4) == 'a');
    BOOST_TEST(((iter + 1 + 1 + 1) - 3 == iter));
    BOOST_TEST(iter[2] == 'o');
    BOOST_TEST(iter[3] == 'b');
    BOOST_TEST(iter[4] == 'a');
    BOOST_TEST(rng[2] == 'o');
    BOOST_TEST(rng[3] == 'b');
    BOOST_TEST(rng[4] == 'a');
    BOOST_TEST(std::ranges::size(rng) == rng.size());
    BOOST_TEST(std::ranges::size(rng) == 9);
    BOOST_TEST(std::ranges::equal(rng, "foobarbaz"sv));
    {
      auto i = iter, j = iter;
      BOOST_TEST((++j == i + 1));
      BOOST_TEST((--j == i));

      BOOST_TEST((j++ == i));
      BOOST_TEST((j == i + 1));

      BOOST_TEST((j-- == i + 1));
      BOOST_TEST((j == i));

      BOOST_TEST(((j += 3) == i + 3));
      BOOST_TEST(((j -= 2) == i + 1));

      BOOST_TEST((i < j));
      BOOST_TEST((i <= j));
      BOOST_TEST((j > i));
      BOOST_TEST((j >= i));
      BOOST_TEST(((i <=> j) < 0));
    }
  }
  // const test
  {
    const auto rng = yk::views::concat("foo"sv, "bar"sv, "baz"sv);
    static_assert(std::ranges::random_access_range<decltype(rng)>);
    std::random_access_iterator auto iter = std::ranges::begin(rng);
    std::random_access_iterator auto sent = std::ranges::end(rng);
    BOOST_TEST((iter != sent));
    BOOST_TEST((sent - iter) == 9);
    BOOST_TEST((iter == iter));
    BOOST_TEST((iter - iter) == 0);
    BOOST_TEST((std::default_sentinel - iter) == 9);
    BOOST_TEST((iter - std::default_sentinel) == -9);
    BOOST_TEST(*(iter + 4) == 'a');
    BOOST_TEST(((iter + 1 + 1 + 1) - 3 == iter));
    BOOST_TEST(iter[2] == 'o');
    BOOST_TEST(iter[3] == 'b');
    BOOST_TEST(iter[4] == 'a');
    BOOST_TEST(rng[2] == 'o');
    BOOST_TEST(rng[3] == 'b');
    BOOST_TEST(rng[4] == 'a');
    BOOST_TEST(std::ranges::size(rng) == rng.size());
    BOOST_TEST(std::ranges::size(rng) == 9);
    BOOST_TEST(std::ranges::equal(rng, "foobarbaz"sv));
    {
      auto i = iter, j = iter;
      BOOST_TEST((++j == i + 1));
      BOOST_TEST((--j == i));

      BOOST_TEST((j++ == i));
      BOOST_TEST((j == i + 1));

      BOOST_TEST((j-- == i + 1));
      BOOST_TEST((j == i));

      BOOST_TEST(((j += 3) == i + 3));
      BOOST_TEST(((j -= 2) == i + 1));

      BOOST_TEST((i < j));
      BOOST_TEST((i <= j));
      BOOST_TEST((j > i));
      BOOST_TEST((j >= i));
      BOOST_TEST(((i <=> j) < 0));
    }
  }
  {
    std::vector<int> vec1;  // random_access_range
    std::vector<int> vec2;  // random_access_range
    auto rng = yk::views::concat(vec1, vec2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::random_access_range<decltype(rng)>);  // concat-ing contiguous_ranges never be contiguous_range
  }
  {
    std::list<int> list1;  // bidirectional_range
    std::list<int> list2;  // bidirectional_range
    auto rng = yk::views::concat(list1, list2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::bidirectional_range<decltype(rng)>);
  }
  {
    std::forward_list<int> forward_list1;  // forward_range
    std::forward_list<int> forward_list2;  // forward_range
    auto rng = yk::views::concat(forward_list1, forward_list2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::forward_range<decltype(rng)>);
  }
  {
    auto input1 = std::views::istream<int>(std::cin);  // input_range
    auto input2 = std::views::istream<int>(std::cin);  // input_range
    auto rng = yk::views::concat(input1, input2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::input_range<decltype(rng)>);
  }
  {
    auto sized1 = std::views::iota(0, 1);  // sized_range
    auto sized2 = std::views::iota(0, 1);  // sized_range
    auto rng = yk::views::concat(sized1, sized2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::sized_range<decltype(rng)>);
  }
  {
    auto&& vec1 = std::vector<int>{};  // viewable_range
    auto&& vec2 = std::vector<int>{};  // viewable_range
    auto rng = yk::views::concat(std::move(vec1), std::move(vec2));
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::viewable_range<decltype(std::move(rng))>);
  }
#if __cpp_lib_ranges_as_const >= 202207L
  {
    auto constant1 = std::views::iota(0, 1);  // constant_range
    auto constant2 = std::views::iota(0, 1);  // constant_range
    auto rng = yk::views::concat(constant1, constant2);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::constant_range<decltype(rng)>);
  }
#endif
  {
    auto not_common = std::views::iota(0);  // arbitrary range
    auto common = std::views::iota(0, 1);   // common_range
    auto rng = yk::views::concat(not_common, common);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::common_range<decltype(rng)>);
  }
  {
    auto view = std::views::istream<int>(std::cin);  // non-common_range with non-copyable iterator
    std::vector<int> vec;                            // common_range with copyable iterator
    auto rng = yk::views::concat(view, vec);
    static_assert(std::ranges::range<decltype(rng)>);  // ill-formed as of P2542R8, but well-formed in our implementation
    // static_assert(std::ranges::common_range<decltype(rng)>);  // in theory, this should be true if above is well-formed, but ill-formed in our implementation
  }
  {
    struct InputIterator {
      using value_type = int;
      using difference_type = int;
      int operator*() const { return 0; }
      InputIterator& operator++() { return *this; }
      InputIterator operator++(int) { return *this; }
    };
    auto input_range = std::ranges::subrange(InputIterator{}, std::unreachable_sentinel);
    static_assert(std::ranges::input_range<decltype(input_range)>);
    auto concat = yk::views::concat(input_range, std::views::iota(0, 1));
    static_assert(std::ranges::range<decltype(concat)>);
  }
  {
    std::vector<int> vec;  // random_access_range
    std::list<int> list;   // bidirectional_range
    auto rng = yk::views::concat(vec, list);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::bidirectional_range<decltype(rng)>);
  }
  {
    std::vector<int> vec;                 // random_access_range
    std::forward_list<int> forward_list;  // forward_range
    auto rng = yk::views::concat(vec, forward_list);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::forward_range<decltype(rng)>);
  }
  {
    std::vector<int> vec;                            // random_access_range
    auto view = std::views::istream<int>(std::cin);  // input_range
    auto rng = yk::views::concat(vec, view);
    static_assert(std::ranges::view<decltype(rng)>);
    static_assert(std::ranges::input_range<decltype(rng)>);
  }

  {
    std::vector a{3, 1, 4, 1, 5};
    std::vector b{9, 2, 6, 5};
    auto view = yk::views::concat(a, b);
    std::ranges::sort(view);
    BOOST_TEST(std::ranges::equal(view, std::vector{1, 1, 2, 3, 4, 5, 5, 6, 9}));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    std::vector b{9, 2, 6, 5};
    std::vector c{3, 5, 8, 9, 7, 9};
    auto view = yk::views::concat(yk::views::concat(a, b), c);
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9}));
  }
  {
    auto view = yk::views::concat(std::views::empty<int>, std::views::empty<int>, std::views::empty<int>);
    BOOST_TEST(std::ranges::empty(view));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    auto view = yk::views::concat(std::views::empty<int>, a, std::views::empty<int>);
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 4, 1, 5}));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    std::vector b{9, 2, 6, 5};
    auto view = yk::views::concat(a, std::views::empty<int>, b);
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 4, 1, 5, 9, 2, 6, 5}));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    auto view = yk::views::concat(a, a);
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 4, 1, 5, 3, 1, 4, 1, 5}));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    auto view = yk::views::concat(a | std::views::take(3), a | std::views::drop(3));
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 4, 1, 5}));
  }
  {
    std::vector a{3, 1, 4, 1, 5};
    auto view = yk::views::concat(a | std::views::take(2), a | std::views::drop(1));
    BOOST_TEST(std::ranges::equal(view, std::vector{3, 1, 1, 4, 1, 5}));
  }
  {
    auto a = yk::views::concat("pen"sv, "pineapple"sv);
    auto b = yk::views::concat("apple"sv, "pen"sv);
    BOOST_TEST(std::ranges::equal(yk::views::concat(a, b), "penpineappleapplepen"sv));
  }
  {
    int a[]{2, 7, 1, 8, 2, 8};
    int b[]{1, 4, 1, 4, 2};
    BOOST_TEST(std::ranges::equal(yk::views::concat(a, b), std::vector{2, 7, 1, 8, 2, 8, 1, 4, 1, 4, 2}));
  }
}

#define YK_CHECK_THROWT(expected, E, ...)   \
  BOOST_REQUIRE_THROW(                      \
      {                                     \
        try {                               \
          yk::throwt<E>(__VA_ARGS__);       \
        } catch (const std::exception& e) { \
          BOOST_TEST(e.what() == expected); \
          throw;                            \
        }                                   \
      },                                    \
      E                                     \
  )

BOOST_AUTO_TEST_CASE(Throwt) {
  class my_exception : public std::runtime_error {
  public:
    my_exception(const std::string& name, const std::string& message) : runtime_error(name + ": " + message) {}
  };

  YK_CHECK_THROWT("foo", std::runtime_error, "foo");
  YK_CHECK_THROWT("foo", std::runtime_error, std::string{"foo"});
  YK_CHECK_THROWT("foo", std::runtime_error, std::string{"foo"}.c_str());
  YK_CHECK_THROWT("foo", std::runtime_error, std::string_view{"foo"});
  YK_CHECK_THROWT("{}", std::runtime_error, "{}");
  YK_CHECK_THROWT("42", std::runtime_error, "{}", 42);
  YK_CHECK_THROWT("foo", std::runtime_error, "{}", "foo");
  YK_CHECK_THROWT("foo", std::runtime_error, "{}", std::string{"foo"});
  YK_CHECK_THROWT("foo", std::runtime_error, "{}", std::string{"foo"}.c_str());
  YK_CHECK_THROWT("foo", std::runtime_error, "{}", std::string_view{"foo"});

  YK_CHECK_THROWT("{}: bar", my_exception, "{}", "bar");
  YK_CHECK_THROWT("foo: bar", my_exception, "foo", "bar");
  
  // YK_CHECK_THROWT("foo: bar", my_exception, "foo", "{}", "bar");   // must be error
  // YK_CHECK_THROWT("{}: foo",  my_exception, "{}", "foo", "bar");   // must be error
  // YK_CHECK_THROWT("foo: bar", my_exception, "foo", "bar", "baz");  // must be error

  // yk::throwt<std::runtime_error>(std::runtime_error("foo"));  // must be error


  // default constructible
  BOOST_REQUIRE_THROW(yk::testing::throw_std_exception(), std::exception);

  // constructible with argument
  YK_CHECK_THROWT("foobar", std::runtime_error, "foobar");

  // constructible with format string
  YK_CHECK_THROWT("33 - 4", std::runtime_error, "{} - {}");

  BOOST_REQUIRE_THROW(
      {
        try {
          yk::testing::throw_system_error(std::make_error_code(std::errc::invalid_argument), std::format("{}", 42).c_str());
        } catch (const std::system_error& e) {
          BOOST_TEST(e.code() == std::make_error_code(std::errc::invalid_argument));
          throw;
        }
      },
      std::system_error
  );

  BOOST_REQUIRE_THROW(
      {
        try {
          yk::testing::throw_system_error(33 - 4, std::generic_category(), std::format("{}", 42).c_str());
        } catch (const std::system_error& e) {
          BOOST_TEST((e.code() == std::error_code{33 - 4, std::generic_category()}));
          throw;
        }
      },
      std::system_error
  );

#if __cpp_lib_print >= 202207L

  try {
    throw std::runtime_error("foobar");
  } catch (const std::exception& e) {
    std::stringstream ss;
    yk::printt(ss, e);
    BOOST_TEST(ss.str() == "foobar\n");
  }

  try {
    try {
      throw std::runtime_error("foo");
    } catch (const std::exception&) {
      std::throw_with_nested(std::runtime_error("bar"));
    }
  } catch (const std::exception& e) {
    std::stringstream ss;
    yk::printt(ss, e);
    BOOST_TEST(ss.str() == "bar\nfoo\n");
  }

#endif

}

BOOST_AUTO_TEST_SUITE_END()  // yk_util
