#include "yk/allocator/default_init_allocator.hpp"
#include "yk/bitmask_enum.hpp"
#include "yk/hash/adapt.hpp"
#include "yk/hash/hash_combine.hpp"
#include "yk/hash/hash_value_for.hpp"
#include "yk/hash/range.hpp"
#include "yk/maybe_mutex.hpp"
#include "yk/par_for_each.hpp"
#include "yk/proxy_hash.hpp"
#include "yk/stack.hpp"
#include "yk/string_hash.hpp"
#include "yk/util/array_cat.hpp"
#include "yk/util/auto_sequence.hpp"
#include "yk/util/forward_like.hpp"
#include "yk/util/pack_indexing.hpp"
#include "yk/util/reverse.hpp"
#include "yk/util/specialization_of.hpp"
#include "yk/util/to_array_of.hpp"
#include "yk/util/to_subrange.hpp"
#include "yk/util/to_subrange/boost.hpp"
#include "yk/util/wrap_as.hpp"

#define BOOST_TEST_MODULE yk_util_test
#if YK_BUILD_UNIT_TEST_FRAMEWORK
#include <boost/test/included/unit_test.hpp>
#else
#include <boost/test/unit_test.hpp>
#endif

#include <boost/container_hash/hash.hpp>
#include <boost/range/iterator_range.hpp>

#include <algorithm>
#include <atomic>
#include <exception>
#include <execution>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include <version>

#include <cstdint>

#if defined(__cpp_lib_forward_like) && !(defined(__clang__) && defined(__GLIBCXX__))
#define YK_UTIL_STD_HAS_FORWARD_LIKE 1
#else
#define YK_UTIL_STD_HAS_FORWARD_LIKE 0
#endif

namespace utf = boost::unit_test;

namespace hash_test {

template <class T, class... Ts>
struct S {
  int val;
};

struct MultiS {
  int a, b, c;
};

}  // namespace hash_test

YK_ADAPT_HASH_TEMPLATE(hash_test, (S<T, Ts...>), val, { return yk::hash_value_for(val.val); }, class T, class... Ts);
YK_ADAPT_HASH(hash_test, MultiS, val, { return yk::hash_combine(val.a, val.b, val.c); });

namespace enum_test {

enum class MyBitmask : std::uint8_t {
  FOO = 1 << 0,
  BAR = 1 << 1,
  BAZ = 1 << 2,
};

enum class SpellType : std::uint8_t {
  TYPE_ATTACK = 1 << 0,
  TYPE_DEFENSE = 1 << 1,

  ATTR_FIRE = 1 << 2,
  ATTR_WATER = 1 << 3,
  ATTR_THUNDER = 1 << 4,
};

}  // namespace enum_test

namespace wrap_as_test {

struct MyData {
  MyData() : str("DC") {}
  MyData(const MyData& other) : value(other.value), str(other.str + "CC") {}
  MyData(MyData&& other) : value(other.value), str(other.str + "MC") {}

  explicit MyData(int value) : value(value), str("IC") {}

  std::string str;
  int value;
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

};  // namespace wrap_as_test

namespace yk {

template <>
struct bitmask_enabled<enum_test::MyBitmask> : std::true_type {
  static enum_test::MyBitmask parse(std::string_view sv) noexcept {
    using enum enum_test::MyBitmask;
    if (sv == "foo") return FOO;
    if (sv == "bar") return BAR;
    if (sv == "baz") return BAZ;
    return {};
  }
};

template <>
struct bitmask_enabled<enum_test::SpellType> : std::true_type {
  static constexpr int min_bit = 2;
  static constexpr int max_bit = 4;
};

}  // namespace yk

BOOST_AUTO_TEST_SUITE(yk_util)

BOOST_AUTO_TEST_CASE(PackIndexing) {
  static_assert(std::is_same_v<yk::pack_indexing_t<0, int, double, std::string>, int>);
  static_assert(std::is_same_v<yk::pack_indexing_t<1, int, double, std::string>, double>);
  static_assert(std::is_same_v<yk::pack_indexing_t<2, int, double, std::string>, std::string>);
}

BOOST_AUTO_TEST_CASE(SpecializationOf) {
  static_assert(yk::specialization_of<std::string, std::basic_string>);
  static_assert(yk::specialization_of<std::vector<int>, std::vector>);

  static_assert(!yk::specialization_of<std::string, std::vector>);
  static_assert(!yk::specialization_of<std::vector<int>, std::basic_string>);
}

#if YK_UTIL_STD_HAS_FORWARD_LIKE
template <class T, class U>
using std_fwd_like_t = decltype(std::forward_like<T>(std::declval<U>()));
#endif

BOOST_AUTO_TEST_CASE(ForwardLike) {
  // clang-format off
  static_assert(std::is_same_v<yk::copy_const_t<      int  ,       float  >,       float  >);
  static_assert(std::is_same_v<yk::copy_const_t<      int  ,       float& >,       float& >);
  static_assert(std::is_same_v<yk::copy_const_t<      int  ,       float&&>,       float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<const int  ,       float  >, const float  >);
  static_assert(std::is_same_v<yk::copy_const_t<const int  ,       float& >, const float& >);
  static_assert(std::is_same_v<yk::copy_const_t<const int  ,       float&&>, const float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<      int& ,       float  >,       float  >);
  static_assert(std::is_same_v<yk::copy_const_t<      int& ,       float& >,       float& >);
  static_assert(std::is_same_v<yk::copy_const_t<      int& ,       float&&>,       float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<const int& ,       float  >, const float  >);
  static_assert(std::is_same_v<yk::copy_const_t<const int& ,       float& >, const float& >);
  static_assert(std::is_same_v<yk::copy_const_t<const int& ,       float&&>, const float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<      int&&,       float  >,       float  >);
  static_assert(std::is_same_v<yk::copy_const_t<      int&&,       float& >,       float& >);
  static_assert(std::is_same_v<yk::copy_const_t<      int&&,       float&&>,       float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<const int&&,       float  >, const float  >);
  static_assert(std::is_same_v<yk::copy_const_t<const int&&,       float& >, const float& >);
  static_assert(std::is_same_v<yk::copy_const_t<const int&&,       float&&>, const float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<      int  , const float  >, const float  >);
  static_assert(std::is_same_v<yk::copy_const_t<      int  , const float& >, const float& >);
  static_assert(std::is_same_v<yk::copy_const_t<      int  , const float&&>, const float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<const int  , const float  >, const float  >);
  static_assert(std::is_same_v<yk::copy_const_t<const int  , const float& >, const float& >);
  static_assert(std::is_same_v<yk::copy_const_t<const int  , const float&&>, const float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<      int& , const float  >, const float  >);
  static_assert(std::is_same_v<yk::copy_const_t<      int& , const float& >, const float& >);
  static_assert(std::is_same_v<yk::copy_const_t<      int& , const float&&>, const float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<const int& , const float  >, const float  >);
  static_assert(std::is_same_v<yk::copy_const_t<const int& , const float& >, const float& >);
  static_assert(std::is_same_v<yk::copy_const_t<const int& , const float&&>, const float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<      int&&, const float  >, const float  >);
  static_assert(std::is_same_v<yk::copy_const_t<      int&&, const float& >, const float& >);
  static_assert(std::is_same_v<yk::copy_const_t<      int&&, const float&&>, const float&&>);
  static_assert(std::is_same_v<yk::copy_const_t<const int&&, const float  >, const float  >);
  static_assert(std::is_same_v<yk::copy_const_t<const int&&, const float& >, const float& >);
  static_assert(std::is_same_v<yk::copy_const_t<const int&&, const float&&>, const float&&>);
  
  static_assert(std::is_same_v<yk::override_ref_t<      int  ,       float  >,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int  ,       float& >,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int  ,       float&&>,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int  ,       float  >,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int  ,       float& >,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int  ,       float&&>,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int& ,       float  >,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int& ,       float& >,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int& ,       float&&>,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int& ,       float  >,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int& ,       float& >,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int& ,       float&&>,       float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int&&,       float  >,       float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<      int&&,       float& >,       float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<      int&&,       float&&>,       float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<const int&&,       float  >,       float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<const int&&,       float& >,       float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<const int&&,       float&&>,       float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<      int  , const float  >, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int  , const float& >, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int  , const float&&>, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int  , const float  >, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int  , const float& >, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int  , const float&&>, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int& , const float  >, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int& , const float& >, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int& , const float&&>, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int& , const float  >, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int& , const float& >, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<const int& , const float&&>, const float& >);
  static_assert(std::is_same_v<yk::override_ref_t<      int&&, const float  >, const float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<      int&&, const float& >, const float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<      int&&, const float&&>, const float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<const int&&, const float  >, const float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<const int&&, const float& >, const float&&>);
  static_assert(std::is_same_v<yk::override_ref_t<const int&&, const float&&>, const float&&>);

  static_assert(std::is_same_v<yk::forward_like_t<      int  ,       float  >,       float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int  ,       float& >,       float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int  ,       float&&>,       float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  ,       float  >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  ,       float& >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  ,       float&&>, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int& ,       float  >,       float& >);
  static_assert(std::is_same_v<yk::forward_like_t<      int& ,       float& >,       float& >);
  static_assert(std::is_same_v<yk::forward_like_t<      int& ,       float&&>,       float& >);
  static_assert(std::is_same_v<yk::forward_like_t<const int& ,       float  >, const float& >);
  static_assert(std::is_same_v<yk::forward_like_t<const int& ,       float& >, const float& >);
  static_assert(std::is_same_v<yk::forward_like_t<const int& ,       float&&>, const float& >);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&,       float  >,       float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&,       float& >,       float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&,       float&&>,       float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&,       float  >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&,       float& >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&,       float&&>, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int  , const float  >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int  , const float& >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int  , const float&&>, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  , const float  >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  , const float& >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  , const float&&>, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int& , const float  >, const float& >);
  static_assert(std::is_same_v<yk::forward_like_t<      int& , const float& >, const float& >);
  static_assert(std::is_same_v<yk::forward_like_t<      int& , const float&&>, const float& >);
  static_assert(std::is_same_v<yk::forward_like_t<const int& , const float  >, const float& >);
  static_assert(std::is_same_v<yk::forward_like_t<const int& , const float& >, const float& >);
  static_assert(std::is_same_v<yk::forward_like_t<const int& , const float&&>, const float& >);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&, const float  >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&, const float& >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&, const float&&>, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&, const float  >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&, const float& >, const float&&>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&, const float&&>, const float&&>);

#if YK_UTIL_STD_HAS_FORWARD_LIKE
  static_assert(std::is_same_v<yk::forward_like_t<      int  ,       float  >, std_fwd_like_t<      int  ,       float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int  ,       float& >, std_fwd_like_t<      int  ,       float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int  ,       float&&>, std_fwd_like_t<      int  ,       float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  ,       float  >, std_fwd_like_t<const int  ,       float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  ,       float& >, std_fwd_like_t<const int  ,       float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  ,       float&&>, std_fwd_like_t<const int  ,       float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<      int& ,       float  >, std_fwd_like_t<      int& ,       float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int& ,       float& >, std_fwd_like_t<      int& ,       float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int& ,       float&&>, std_fwd_like_t<      int& ,       float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<const int& ,       float  >, std_fwd_like_t<const int& ,       float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int& ,       float& >, std_fwd_like_t<const int& ,       float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int& ,       float&&>, std_fwd_like_t<const int& ,       float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&,       float  >, std_fwd_like_t<      int&&,       float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&,       float& >, std_fwd_like_t<      int&&,       float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&,       float&&>, std_fwd_like_t<      int&&,       float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&,       float  >, std_fwd_like_t<const int&&,       float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&,       float& >, std_fwd_like_t<const int&&,       float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&,       float&&>, std_fwd_like_t<const int&&,       float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<      int  , const float  >, std_fwd_like_t<      int  , const float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int  , const float& >, std_fwd_like_t<      int  , const float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int  , const float&&>, std_fwd_like_t<      int  , const float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  , const float  >, std_fwd_like_t<const int  , const float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  , const float& >, std_fwd_like_t<const int  , const float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int  , const float&&>, std_fwd_like_t<const int  , const float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<      int& , const float  >, std_fwd_like_t<      int& , const float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int& , const float& >, std_fwd_like_t<      int& , const float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int& , const float&&>, std_fwd_like_t<      int& , const float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<const int& , const float  >, std_fwd_like_t<const int& , const float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int& , const float& >, std_fwd_like_t<const int& , const float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int& , const float&&>, std_fwd_like_t<const int& , const float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&, const float  >, std_fwd_like_t<      int&&, const float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&, const float& >, std_fwd_like_t<      int&&, const float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<      int&&, const float&&>, std_fwd_like_t<      int&&, const float&&>>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&, const float  >, std_fwd_like_t<const int&&, const float  >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&, const float& >, std_fwd_like_t<const int&&, const float& >>);
  static_assert(std::is_same_v<yk::forward_like_t<const int&&, const float&&>, std_fwd_like_t<const int&&, const float&&>>);
#endif
  // clang-format on
}

BOOST_AUTO_TEST_CASE(ToArrayOf) {
  static_assert(yk::to_array_of<int>(3, 1, 4, 1, 5) == std::array{3, 1, 4, 1, 5});
  static_assert(yk::to_array_of<double>(3, 1, 4, 1, 5) == std::array{3.0, 1.0, 4.0, 1.0, 5.0});

  enum class StrongID : int {};
  constexpr auto predefined_ids = yk::to_array_of<StrongID>(1, 2, 3);
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

BOOST_AUTO_TEST_CASE(ToSubrange) {
  std::set<int> s{3, 1, 4, 1, 5};

  BOOST_TEST((std::ranges::equal(std::vector<int>{1}, yk::to_subrange(s.equal_range(1)))));
  BOOST_TEST((std::ranges::equal(std::vector<int>{}, yk::to_subrange(s.equal_range(2)))));

  std::vector v{3, 1, 4, 1, 5};
  auto rng = boost::make_iterator_range(v);
  BOOST_TEST((std::ranges::equal(v, yk::to_subrange(rng))));
}

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

#if __cpp_lib_parallel_algorithm >= 201603L

BOOST_AUTO_TEST_CASE(ParForEach) {
  std::vector vec{3, 1, 4, 1, 5};
  std::atomic<int> sum;
  const auto non_throw_func = [&](int x) { sum += x; };
  const auto throw_func = [&](int x) {
    if (x == 4) throw std::runtime_error("");
    sum += x;
  };

  std::for_each(std::execution::seq, vec.begin(), vec.end(), non_throw_func);

  {
    sum = 0;
    yk::ranges::for_each(yk::execution::abort, std::execution::seq, vec, non_throw_func);
    BOOST_TEST((sum == 14));
  }
  {
    sum = 0;
    BOOST_REQUIRE_THROW(yk::ranges::for_each(yk::execution::abort, std::execution::seq, vec, throw_func), std::runtime_error);
    BOOST_TEST((sum < 14));
  }
  {
    sum = 0;
    yk::ranges::for_each(yk::execution::proceed, std::execution::seq, vec, non_throw_func);
    BOOST_TEST((sum == 14));
  }
  {
    sum = 0;
    BOOST_REQUIRE_THROW(yk::ranges::for_each(yk::execution::proceed, std::execution::seq, vec, throw_func), std::runtime_error);
    BOOST_TEST((sum == 10));
  }
}

BOOST_AUTO_TEST_CASE(MaybeMutex) {
  static_assert(yk::xo::Lockable<yk::maybe_mutex<std::mutex, std::execution::parallel_policy>>);
  static_assert(yk::xo::Lockable<yk::maybe_mutex<std::mutex, std::execution::sequenced_policy>>);
}

#endif  // __cpp_lib_parallel_algorithm

BOOST_AUTO_TEST_CASE(Hash) {
  BOOST_TEST(std::hash<int>{}(42) == yk::std_hash_value_for(42));
  BOOST_TEST(boost::hash<int>{}(42) == yk::boost_hash_value_for(42));

  hash_test::S<int, double> s{42};
  BOOST_TEST(yk::hash_value_for(s) == yk::hash_value_for(42));
  BOOST_TEST(hash_value(s) == yk::hash_value_for(42));  // call hash_value by ADL

  {
    hash_test::MultiS s{31415, 9265, 3589};
    std::size_t seed = yk::hash_value_for(s.a);
    boost::hash_combine(seed, yk::hash_value_for(s.b));
    boost::hash_combine(seed, yk::hash_value_for(s.c));
    BOOST_TEST(hash_value(s) == seed);
  }
}

BOOST_AUTO_TEST_CASE(ProxyHash) {
  struct S {
    int value;
    int get_value() const { return value; }
    constexpr bool operator==(const S&) const noexcept = default;
  };
  BOOST_TEST((yk::proxy_hash<S, &S::value>{}(S{42}) == std::hash<int>{}(42)));
  BOOST_TEST((yk::proxy_hash<S, &S::get_value>{}(S{42}) == std::hash<int>{}(42)));
  BOOST_TEST((yk::proxy_hash<S, [](const S& s) { return s.value; }>{}(S{42}) == std::hash<int>{}(42)));

  std::unordered_set<S, yk::proxy_hash<S, &S::value>, std::equal_to<>> set;
  set.insert(S{42});
  BOOST_TEST(set.contains(S{42}));
}

BOOST_AUTO_TEST_CASE(StringHash) {
  using namespace std::literals;
  BOOST_TEST(yk::string_hash{}("123") == yk::string_hash{}("123"sv));
  BOOST_TEST(yk::string_hash{}("123") == yk::string_hash{}("123"s));

  std::unordered_set<std::string, yk::string_hash, std::equal_to<>> set;
  set.insert("foo");
  BOOST_TEST(set.contains("foo"));
  BOOST_TEST(set.contains("foo"s));
  BOOST_TEST(set.contains("foo"sv));
}

BOOST_AUTO_TEST_CASE(RangeHash) {
  std::vector vec{3, 1, 4, 1, 5};
  BOOST_TEST(yk::hash_range(vec) == yk::hash_combine(3, 1, 4, 1, 5));
  BOOST_TEST(yk::hash_combine(33, vec, 4) == yk::hash_combine(33, 3, 1, 4, 1, 5, 4));
}

BOOST_AUTO_TEST_CASE(Enum) {
  using namespace enum_test;
  using namespace yk::bitmask_operators;

  using enum MyBitmask;

  BOOST_TEST((~FOO == static_cast<MyBitmask>(~yk::to_underlying(FOO))));

  BOOST_TEST(((FOO & BAR) == static_cast<MyBitmask>(yk::to_underlying(FOO) & yk::to_underlying(BAR))));
  BOOST_TEST(((FOO ^ BAR) == static_cast<MyBitmask>(yk::to_underlying(FOO) ^ yk::to_underlying(BAR))));
  BOOST_TEST(((FOO | BAR) == static_cast<MyBitmask>(yk::to_underlying(FOO) | yk::to_underlying(BAR))));

  // clang-format off

  BOOST_TEST( yk::contains(FOO, FOO));
  BOOST_TEST(!yk::contains(FOO, BAR));
  BOOST_TEST( yk::contains(FOO | BAR, FOO));
  BOOST_TEST(!yk::contains(FOO | BAR, BAZ));
  BOOST_TEST(!yk::contains(FOO |       BAZ, FOO | BAR));
  BOOST_TEST(!yk::contains(      BAR | BAZ, FOO | BAR));
  BOOST_TEST( yk::contains(FOO | BAR | BAZ, FOO | BAR));

  BOOST_TEST( yk::contains_any_bit(FOO, FOO));
  BOOST_TEST(!yk::contains_any_bit(FOO, BAR));
  BOOST_TEST( yk::contains_any_bit(FOO | BAR, FOO));
  BOOST_TEST(!yk::contains_any_bit(FOO | BAR, BAZ));
  BOOST_TEST( yk::contains_any_bit(FOO |       BAZ, FOO | BAR));
  BOOST_TEST( yk::contains_any_bit(      BAR | BAZ, FOO | BAR));
  BOOST_TEST( yk::contains_any_bit(FOO | BAR | BAZ, FOO | BAR));

  BOOST_TEST( yk::contains_single_bit(FOO, FOO));
  BOOST_TEST(!yk::contains_single_bit(FOO, BAR));
  BOOST_TEST( yk::contains_single_bit(FOO | BAR, FOO));
  BOOST_TEST(!yk::contains_single_bit(FOO | BAR, BAZ));

  // clang-format on

  BOOST_TEST((yk::parse_flag<MyBitmask>("foo") == FOO));
  BOOST_TEST((yk::parse_flag<MyBitmask>("bar") == BAR));
  BOOST_TEST((yk::parse_flag<MyBitmask>("baz") == BAZ));
  BOOST_TEST((yk::parse_flag<MyBitmask>("yay") == MyBitmask{}));

  BOOST_TEST((yk::parse_flags<MyBitmask>("foo", "|") == FOO));
  BOOST_TEST((yk::parse_flags<MyBitmask>("yay", "|") == MyBitmask{}));

  BOOST_TEST((yk::parse_flags<MyBitmask>("foo|bar", "|") == (FOO | BAR)));
  BOOST_TEST((yk::parse_flags<MyBitmask>("foo|yay", "|") == MyBitmask{}));
  BOOST_TEST((yk::parse_flags<MyBitmask>("foo,bar", "|") == MyBitmask{}));

  BOOST_TEST(std::ranges::equal(yk::each_bit(SpellType::TYPE_ATTACK | SpellType::ATTR_FIRE | SpellType::ATTR_THUNDER),
                                std::vector{SpellType::ATTR_FIRE, SpellType::ATTR_THUNDER}));

  BOOST_TEST(std::ranges::equal(yk::each_bit(static_cast<SpellType>(static_cast<std::underlying_type_t<SpellType>>(-1))),
                                std::vector{SpellType::ATTR_FIRE, SpellType::ATTR_WATER, SpellType::ATTR_THUNDER}));
}

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

  BOOST_TEST([&]<auto... Vals>(const S& s, yk::auto_sequence<Vals...>) {  //
    return (use_member.template operator()<Vals>(s) + ...);
  }(S{33, 9}, yk::auto_sequence<&S::a, &S::b>{}) == 42);
}

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

  using namespace wrap_as_test;

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

BOOST_AUTO_TEST_SUITE_END()  // yk_util
