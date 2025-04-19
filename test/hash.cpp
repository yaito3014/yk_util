#include "yk/hash/adapt.hpp"
#include "yk/hash/hash_combine.hpp"
#include "yk/hash/hash_value_for.hpp"
#include "yk/hash/proxy_hash.hpp"
#include "yk/hash/range.hpp"
#include "yk/hash/string_hash.hpp"

#include <boost/container_hash/hash.hpp>
#include <boost/test/unit_test.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include <cstddef>

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

BOOST_AUTO_TEST_SUITE(hash)
BOOST_AUTO_TEST_CASE(Hash) {
  BOOST_TEST(std::hash<int>{}(42) == yk::std_hash_value_for(42));
  BOOST_TEST(boost::hash<int>{}(42) == yk::boost_hash_value_for(42));

  hash_test::S<int, double> s{42};
  BOOST_TEST(yk::hash_value_for(s) == yk::hash_value_for(42));
  BOOST_TEST(hash_value(s) == yk::hash_value_for(42));  // call hash_value by ADL

  {
    hash_test::MultiS ms{31415, 9265, 3589};
    std::size_t seed = 0;
    boost::hash_combine(seed, boost::hash<int>{}(ms.a));
    boost::hash_combine(seed, boost::hash<int>{}(ms.b));
    boost::hash_combine(seed, boost::hash<int>{}(ms.c));
    BOOST_TEST(hash_value(ms) == seed);
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
  BOOST_TEST(yk::hash_combine(33, vec, 4) == yk::hash_combine(33, yk::hash_combine(3, 1, 4, 1, 5), 4));
}

BOOST_AUTO_TEST_SUITE_END()
