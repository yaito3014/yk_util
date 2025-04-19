#include "yk/util/array_cat.hpp"
#include "yk/util/auto_sequence.hpp"
#include "yk/util/reverse.hpp"
#include "yk/util/to_array_of.hpp"
#include "yk/util/to_subrange.hpp"
#include "yk/util/to_subrange/boost.hpp"
#include "yk/util/wrap_as.hpp"

#include <boost/range/iterator_range.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <array>
#include <set>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

BOOST_AUTO_TEST_SUITE(util)

BOOST_AUTO_TEST_CASE(ArrayCat) {
  BOOST_TEST((yk::array_cat(std::array<int, 0>{}, std::array<int, 0>{}) == std::array<int, 0>{}));
  BOOST_TEST((yk::array_cat(std::array<int, 0>{}, std::array{3, 1, 4}) == std::array{3, 1, 4}));
  BOOST_TEST((yk::array_cat(std::array{3, 1, 4}, std::array<int, 0>{}) == std::array{3, 1, 4}));
  BOOST_TEST((yk::array_cat(std::array{3, 1, 4}, std::array{1, 5, 9, 2}, std::array{6, 5}) == std::array{3, 1, 4, 1, 5, 9, 2, 6, 5}));
}

BOOST_AUTO_TEST_CASE(AutoSeq) {
  struct S {
    int a, b;
  };

  const auto use_member = []<auto Mem>(const S& s) { return s.*Mem; };

  BOOST_TEST([&]<auto... Vals>(const S& s, yk::auto_sequence<Vals...>) {
    return (use_member.template operator()<Vals>(s) + ...);
  }(S{33, 9}, yk::auto_sequence<&S::a, &S::b>{}) == 42);
}

BOOST_AUTO_TEST_CASE(Reverse) {
  static_assert(yk::reverse(42) < yk::reverse(33 - 4));
  static_assert(yk::reverse(42) == yk::reverse(42));

  struct S {
    int a, b, c;
    constexpr bool operator==(const S&) const noexcept = default;
  };

  static_assert(std::ranges::equal(
      []() {
        std::vector<S> vec{{3, 1, 4}, {1, 5, 9}, {2, 6, 5}, {3, 5, 8}, {9, 7, 9}, {3, 2, 3}, {8, 4, 6}};
        std::ranges::sort(
            vec, [](const S& x, const S& y) { return std::forward_as_tuple(x.a, yk::reverse(x.b), x.c) < std::forward_as_tuple(y.a, yk::reverse(y.b), y.c); });
        return vec;
      }(),
      std::vector<S>{{1, 5, 9}, {2, 6, 5}, {3, 5, 8}, {3, 2, 3}, {3, 1, 4}, {8, 4, 6}, {9, 7, 9}}));

  static_assert(std::ranges::equal(
      []() {
        std::vector<S> vec{{3, 1, 4}, {1, 5, 9}, {2, 6, 5}, {3, 5, 8}, {9, 7, 9}, {3, 2, 3}, {8, 4, 6}};
        std::ranges::sort(vec, {}, [](const S& s) { return std::make_tuple(std::cref(s.a), yk::reverse(s.b), std::cref(s.c)); });
        return vec;
      }(),
      std::vector<S>{{1, 5, 9}, {2, 6, 5}, {3, 5, 8}, {3, 2, 3}, {3, 1, 4}, {8, 4, 6}, {9, 7, 9}}));
}

BOOST_AUTO_TEST_CASE(ToArrayOf) {
  static_assert(yk::to_array_of<int>(3, 1, 4, 1, 5) == std::array{3, 1, 4, 1, 5});
  static_assert(yk::to_array_of<double>(3, 1, 4, 1, 5) == std::array{3.0, 1.0, 4.0, 1.0, 5.0});

  enum class StrongID : int {};
  [[maybe_unused]] constexpr auto predefined_ids = yk::to_array_of<StrongID>(1, 2, 3);
}

BOOST_AUTO_TEST_CASE(ToSubrange) {
  std::set<int> s{3, 1, 4, 1, 5};

  BOOST_TEST((std::ranges::equal(std::vector<int>{1}, yk::to_subrange(s.equal_range(1)))));
  BOOST_TEST((std::ranges::equal(std::vector<int>{}, yk::to_subrange(s.equal_range(2)))));

  std::vector v{3, 1, 4, 1, 5};
  auto rng = boost::make_iterator_range(v);
  BOOST_TEST((std::ranges::equal(v, yk::to_subrange(rng))));
}

struct MyData {
  MyData() : str("DC") {}
  MyData(const MyData& other) : value(other.value), str(other.str + "CC") {}
  MyData(MyData&& other) : value(other.value), str(other.str + "MC") {}

  explicit MyData(int value) : value(value), str("IC") {}

  int value;
  std::string str;
};

struct ID {
  explicit ID(const MyData& data) : value("id_" + std::to_string(data.value) + "_" + data.str) {}

  std::string value;
};

struct MyClassConstruct {
  template <class T>
  explicit MyClassConstruct(T&& data) : id{MyData{std::forward<T>(data)}} {}

  ID id;
};

struct MyClassWrap {
  template <class T>
  explicit MyClassWrap(T&& data) : id{yk::wrap_as<MyData>(std::forward<T>(data))} {}

  ID id;
};

BOOST_AUTO_TEST_CASE(WrapAs) {
  // clang-format off
  static_assert(std::is_same_v<yk::wrap_as_t<int,       int  >,       int&&>);
  static_assert(std::is_same_v<yk::wrap_as_t<int,       int& >,       int& >);
  static_assert(std::is_same_v<yk::wrap_as_t<int,       int&&>,       int&&>);
  static_assert(std::is_same_v<yk::wrap_as_t<int, const int  >, const int&&>);
  static_assert(std::is_same_v<yk::wrap_as_t<int, const int& >, const int&>);
  static_assert(std::is_same_v<yk::wrap_as_t<int, const int&&>, const int&&>);

  static_assert(std::is_same_v<yk::wrap_as_t<int,       float  >, int>);
  static_assert(std::is_same_v<yk::wrap_as_t<int,       float& >, int>);
  static_assert(std::is_same_v<yk::wrap_as_t<int,       float&&>, int>);
  static_assert(std::is_same_v<yk::wrap_as_t<int, const float  >, int>);
  static_assert(std::is_same_v<yk::wrap_as_t<int, const float& >, int>);
  static_assert(std::is_same_v<yk::wrap_as_t<int, const float&&>, int>);
  // clang-format on

  MyData data{42};

  static_assert(std::is_same_v<decltype(yk::wrap_as<MyData>(42)), MyData>);

  static_assert(std::is_same_v<decltype(yk::wrap_as<MyData>(MyData{42})), MyData&&>);
  static_assert(std::is_same_v<decltype(yk::wrap_as<MyData>(yk::wrap_as<MyData>(42))), MyData&&>);

  static_assert(std::is_same_v<decltype(yk::wrap_as<MyData>(data)), MyData&>);
  static_assert(std::is_same_v<decltype(yk::wrap_as<MyData>(yk::wrap_as<MyData>(data))), MyData&>);

  static_assert(std::is_same_v<decltype(yk::wrap_as<MyData>(std::as_const(data))), const MyData&>);
  static_assert(std::is_same_v<decltype(yk::wrap_as<MyData>(yk::wrap_as<MyData>(std::as_const(data)))), const MyData&>);

  {
    auto&& val_naive = MyData{42};
    auto&& val_wrap = yk::wrap_as<MyData>(42);
    BOOST_TEST(val_naive.str == "IC");
    BOOST_TEST(val_wrap.str == "IC");
  }
  {
    auto&& val_naive = MyData(data);
    auto&& val_wrap = yk::wrap_as<MyData>(data);
    BOOST_TEST(val_naive.str == "ICCC");
    BOOST_TEST(val_wrap.str == "IC");
  }
  {
    auto val_naive = MyData{MyData{42}};
    // auto val_wrap = yk::wrap_as<MyData>(MyData{42});  // dangling reference
    // auto val_fwd = yk::forward<MyData>(MyData{42});   // dangling reference
    BOOST_TEST(val_naive.str == "IC");
    // BOOST_TEST(val_wrap.str == "IC"); // UB
    // BOOST_TEST(val_fwd.str == "IC");  // UB
  }
  {
    auto&& val = MyClassConstruct(42);
    BOOST_TEST(val.id.value == "id_42_IC");
  }
  {
    auto&& val = MyClassConstruct(MyData{42});
    BOOST_TEST(val.id.value == "id_42_ICMC");
  }
  {
    auto&& val = MyClassConstruct(data);
    BOOST_TEST(val.id.value == "id_42_ICCC");
  }
  {
    auto&& val = MyClassWrap(42);
    BOOST_TEST(val.id.value == "id_42_IC");
  }
  {
    auto&& val = MyClassWrap(MyData{42});
    BOOST_TEST(val.id.value == "id_42_IC");
  }
  {
    auto&& val = MyClassWrap(data);
    BOOST_TEST(val.id.value == "id_42_IC");
  }
}

BOOST_AUTO_TEST_SUITE_END()
