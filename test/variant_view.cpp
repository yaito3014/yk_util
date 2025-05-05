#define YK_VARIANT_INCLUDE_STD 1
#define YK_VARIANT_INCLUDE_BOOST 1
#include "yk/util/overloaded.hpp"
#include "yk/util/specialization_of.hpp"

#include "yk/variant/boost/compare.hpp"
#include "yk/variant_view.hpp"
#include "yk/variant_view/hash.hpp"

#include <boost/test/unit_test.hpp>

#include <boost/core/ignore_unused.hpp>
#include <boost/utility/identity_type.hpp>
#include <boost/variant.hpp>
#include <boost/variant/recursive_variant.hpp>

#include <concepts>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(variant_view)

template <class... Ts>
using variant_t =
    std::tuple<std::variant<Ts...>, boost::variant<Ts...>, typename boost::make_recursive_variant<Ts..., std::vector<boost::recursive_variant_>>::type>;

#define YK_VARIANT(...) BOOST_IDENTITY_TYPE((variant_t<__VA_ARGS__>))

BOOST_AUTO_TEST_CASE_TEMPLATE(Initialization, Variant, YK_VARIANT(int, double)) {
  Variant v = 42;

  // identical sets (original variant set == view set)
  {
    auto view = yk::make_variant_view<int, double>(v);
    static_assert(std::same_as<decltype(view), yk::variant_view<Variant, int, double>>);
  }
  {
    auto view = yk::make_variant_view<int, double>(std::as_const(v));
    static_assert(std::same_as<decltype(view), yk::variant_view<const Variant, int, double>>);
  }
  {
    static_assert(std::is_constructible_v<yk::variant_view<Variant, int, double>, Variant&&>);
    static_assert(std::same_as<decltype(yk::make_variant_view<int, double>(Variant{42})), yk::variant_view<Variant, int, double>>);
  }

  // clang-format off
  // direct initialization (variant_view view{variant};)
  {
    static_assert(std::is_nothrow_constructible_v<yk::variant_view<      Variant, int, double>, Variant>);
    static_assert(std::is_nothrow_constructible_v<yk::variant_view<const Variant, int, double>, Variant>);

    static_assert(std::is_nothrow_constructible_v<yk::variant_view<      Variant, int, double>, Variant&>);
    static_assert(std::is_nothrow_constructible_v<yk::variant_view<const Variant, int, double>, Variant&>);

    static_assert(!std::is_nothrow_constructible_v<yk::variant_view<     Variant, int, double>, const Variant&>);
    static_assert(std::is_nothrow_constructible_v<yk::variant_view<const Variant, int, double>, const Variant&>);

    static_assert(std::is_nothrow_constructible_v<yk::variant_view<      Variant, int, double>, Variant&&>);
    static_assert(std::is_nothrow_constructible_v<yk::variant_view<const Variant, int, double>, Variant&&>);
  }

  // trivial functions
  {
    static_assert(std::is_trivially_copyable_v<yk::variant_view<      Variant, int, double>>);
    static_assert(std::is_trivially_copyable_v<yk::variant_view<const Variant, int, double>>);

    static_assert(std::is_nothrow_default_constructible_v<yk::variant_view<      Variant, int, double>>);
    static_assert(std::is_nothrow_default_constructible_v<yk::variant_view<const Variant, int, double>>);

    static_assert(std::is_trivially_copy_constructible_v<yk::variant_view<      Variant, int, double>>);
    static_assert(std::is_trivially_copy_constructible_v<yk::variant_view<const Variant, int, double>>);

    static_assert(std::is_nothrow_copy_constructible_v<yk::variant_view<      Variant, int, double>>);
    static_assert(std::is_nothrow_copy_constructible_v<yk::variant_view<const Variant, int, double>>);

    static_assert(std::is_trivially_move_constructible_v<yk::variant_view<      Variant, int, double>>);
    static_assert(std::is_trivially_move_constructible_v<yk::variant_view<const Variant, int, double>>);

    static_assert(std::is_nothrow_move_constructible_v<yk::variant_view<      Variant, int, double>>);
    static_assert(std::is_nothrow_move_constructible_v<yk::variant_view<const Variant, int, double>>);

    static_assert(std::is_trivially_copy_assignable_v<yk::variant_view<      Variant, int, double>>);
    static_assert(std::is_trivially_copy_assignable_v<yk::variant_view<const Variant, int, double>>);

    static_assert(std::is_nothrow_copy_assignable_v<yk::variant_view<      Variant, int, double>>);
    static_assert(std::is_nothrow_copy_assignable_v<yk::variant_view<const Variant, int, double>>);

    static_assert(std::is_trivially_move_assignable_v<yk::variant_view<      Variant, int, double>>);
    static_assert(std::is_trivially_move_assignable_v<yk::variant_view<const Variant, int, double>>);

    static_assert(std::is_nothrow_move_assignable_v<yk::variant_view<      Variant, int, double>>);
    static_assert(std::is_nothrow_move_assignable_v<yk::variant_view<const Variant, int, double>>);

    static_assert( std::is_nothrow_convertible_v<yk::variant_view<      Variant, int, double>, yk::variant_view<const Variant, int, double>>);
    static_assert(!std::is_nothrow_convertible_v<yk::variant_view<const Variant, int, double>, yk::variant_view<      Variant, int, double>>);
  }
  // clang-format on

  // view copying
  {
    auto mutable_view = yk::make_variant_view<int, double>(v);

    static_assert(std::is_constructible_v<yk::variant_view<Variant, int, double>, decltype(mutable_view)>);
    static_assert(std::is_constructible_v<yk::variant_view<const Variant, int, double>, decltype(mutable_view)>);
  }

  BOOST_TEST((yk::variant_view<Variant, int, double>{}.invalid()));
  BOOST_TEST((yk::variant_view<const Variant, int, double>{}.invalid()));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(SubView, Variant, YK_VARIANT(int, float, double)) {
  Variant v = 3.14;

  // trivial
  {
    // adding const
    [[maybe_unused]] yk::variant_view<const Variant, int, float> const_view = yk::make_variant_view(v).template subview<int, float>();
  }

  {
    // mutable version
    {
      auto view = yk::variant_view<Variant, int, float, double>(v);

      auto int_float_double_view = view.template subview<int, float, double>();
      static_assert(std::same_as<decltype(int_float_double_view), yk::variant_view<Variant, int, float, double>>);

      auto int_float_double_view2 = int_float_double_view.template subview<int, float, double>();
      static_assert(std::same_as<decltype(int_float_double_view2), decltype(int_float_double_view)>);

      auto int_float_view = view.template subview<int, float>();
      static_assert(std::same_as<decltype(int_float_view), yk::variant_view<Variant, int, float>>);

      auto int_view = int_float_view.template subview<int>();
      static_assert(std::same_as<decltype(int_view), yk::variant_view<Variant, int>>);
    }

    // const version
    {
      auto view = yk::variant_view<const Variant, int, float, double>(v);

      auto int_float_double_view = view.template subview<int, float, double>();
      static_assert(std::same_as<decltype(int_float_double_view), yk::variant_view<const Variant, int, float, double>>);

      auto int_float_double_view2 = int_float_double_view.template subview<int, float, double>();
      static_assert(std::same_as<decltype(int_float_double_view2), decltype(int_float_double_view)>);

      auto int_float_view = view.template subview<int, float>();
      static_assert(std::same_as<decltype(int_float_view), yk::variant_view<const Variant, int, float>>);

      auto int_view = int_float_view.template subview<int>();
      static_assert(std::same_as<decltype(int_view), yk::variant_view<const Variant, int>>);
    }
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Visit, Variant, YK_VARIANT(int, double, std::string)) {
  // we are checking for potential implicit type conversion in this test...

  {
    const auto do_visit = [&](const auto& visitable) {
      return yk::visit(yk::overloaded{
                           [](const int&) -> std::string { return "int"; },
                           [](const double&) -> std::string { return "double"; },
                           [](const std::string&) -> std::string { return "string"; },
                           [](const std::vector<Variant>&) -> std::string { return "vector"; },
                       },
                       visitable);
    };

    BOOST_TEST(do_visit(Variant{42}) == "int");
    BOOST_TEST(do_visit(Variant{3.14}) == "double");
    BOOST_TEST(do_visit(Variant{std::string{"foo"}}) == "string");

    BOOST_TEST(do_visit(yk::variant_view<const Variant, int, double, std::string>{Variant{42}}) == "int");
    BOOST_TEST(do_visit(yk::variant_view<const Variant, int, double, std::string>{Variant{3.14}}) == "double");

    BOOST_REQUIRE_THROW(do_visit(yk::variant_view<const Variant, int, double, std::string>{}), yk::uninitialized_variant_view);
  }
  {
    const auto do_visit_with_R = [&](const auto& visitable) {
      return yk::visit<std::string>(yk::overloaded{
                                        [](const int&) -> std::string { return "int"; },
                                        [](const double&) -> const char* { return "double"; },
                                        [](const std::string&) -> const char* { return "string"; },
                                        [](const std::vector<Variant>&) -> std::string { return "vector"; },
                                    },
                                    visitable);
    };

    BOOST_TEST(do_visit_with_R(Variant{42}) == "int");
    BOOST_TEST(do_visit_with_R(Variant{3.14}) == "double");
    BOOST_TEST(do_visit_with_R(Variant{std::string{"foo"}}) == "string");

    BOOST_TEST(do_visit_with_R(yk::variant_view<const Variant, int, double, std::string>{Variant{42}}) == "int");
    BOOST_TEST(do_visit_with_R(yk::variant_view<const Variant, int, double, std::string>{Variant{3.14}}) == "double");
    BOOST_TEST(do_visit_with_R(yk::variant_view<const Variant, int, double, std::string>{Variant{std::string{"foo"}}}) == "string");

    BOOST_REQUIRE_THROW(do_visit_with_R(yk::variant_view<const Variant, int, double, std::string>{}), yk::uninitialized_variant_view);
  }

  // visiting subviews, with fully exhaustive visitor
  {
    const auto do_visit = [&](const auto& visitable) {
      return yk::visit(yk::overloaded{
                           [](const int&) -> std::string { return "int"; },
                           [](const double&) -> std::string { return "double"; },
                           [](const std::string&) -> std::string { return "string"; },
                       },
                       visitable);
    };

    BOOST_REQUIRE_THROW(boost::ignore_unused(do_visit(yk::make_variant_view(Variant{42}).template subview<double, std::string>()) == "int"),
                        std::bad_variant_access);

    BOOST_TEST(do_visit(yk::make_variant_view(Variant{3.14}).template subview<double, std::string>()) == "double");
    BOOST_TEST(do_visit(yk::make_variant_view(Variant{std::string{"foo"}}).template subview<double, std::string>()) == "string");
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(HoldAlternative, Variant, YK_VARIANT(int, double, std::string)) {
  BOOST_TEST(yk::holds_alternative<int>(Variant{42}));
  BOOST_TEST(yk::holds_alternative<double>(Variant{3.14}));
  BOOST_TEST(yk::holds_alternative<std::string>(Variant{"foo"}));

  BOOST_TEST(!yk::holds_alternative<double>(Variant{42}));
  BOOST_TEST(!yk::holds_alternative<std::string>(Variant{3.14}));
  BOOST_TEST(!yk::holds_alternative<int>(Variant{"foo"}));

  BOOST_TEST(yk::holds_alternative<int>(yk::variant_view<Variant, int, double, std::string>(Variant{42})));
  BOOST_TEST(yk::holds_alternative<double>(yk::variant_view<Variant, int, double, std::string>(Variant{3.14})));
  BOOST_TEST(yk::holds_alternative<std::string>(yk::variant_view<Variant, int, double, std::string>(Variant{"foo"})));

  BOOST_TEST(!yk::holds_alternative<double>(yk::variant_view<Variant, int, double, std::string>(Variant{42})));
  BOOST_TEST(!yk::holds_alternative<std::string>(yk::variant_view<Variant, int, double, std::string>(Variant{3.14})));
  BOOST_TEST(!yk::holds_alternative<int>(yk::variant_view<Variant, int, double, std::string>(Variant{"foo"})));

  BOOST_TEST(yk::holds_alternative<int>(yk::variant_view<const Variant, int, double, std::string>(Variant{42})));
  BOOST_TEST(yk::holds_alternative<double>(yk::variant_view<const Variant, int, double, std::string>(Variant{3.14})));
  BOOST_TEST(yk::holds_alternative<std::string>(yk::variant_view<const Variant, int, double, std::string>(Variant{"foo"})));

  BOOST_TEST(!yk::holds_alternative<double>(yk::variant_view<const Variant, int, double, std::string>(Variant{42})));
  BOOST_TEST(!yk::holds_alternative<std::string>(yk::variant_view<const Variant, int, double, std::string>(Variant{3.14})));
  BOOST_TEST(!yk::holds_alternative<int>(yk::variant_view<const Variant, int, double, std::string>(Variant{"foo"})));

  BOOST_TEST(!yk::holds_alternative<std::string>(yk::variant_view<Variant, int, double, std::string>{}));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Get, Variant, YK_VARIANT(int, double, std::string)) {
  BOOST_TEST(yk::get<int>(Variant{42}) == 42);
  BOOST_TEST(yk::get<double>(Variant{3.14}) == 3.14);
  BOOST_TEST(yk::get<std::string>(Variant{"foo"}) == "foo");

  BOOST_TEST(yk::get<int>(yk::variant_view<Variant, int, double>{Variant{42}}) == 42);
  BOOST_TEST(yk::get<double>(yk::variant_view<Variant, int, double>{Variant{3.14}}) == 3.14);

  BOOST_TEST(yk::get<int>(yk::variant_view<const Variant, int, double>{Variant{42}}) == 42);
  BOOST_TEST(yk::get<double>(yk::variant_view<const Variant, int, double>{Variant{3.14}}) == 3.14);

  BOOST_TEST(yk::get<double>(yk::make_variant_view(Variant{3.14}).template subview<double, std::string>()) == 3.14);
  BOOST_TEST(yk::get<std::string>(yk::make_variant_view(Variant{"foo"}).template subview<double, std::string>()) == "foo");

  BOOST_TEST(yk::get<0>(Variant{42}) == 42);
  BOOST_TEST(yk::get<1>(Variant{3.14}) == 3.14);
  BOOST_TEST(yk::get<2>(Variant{"foo"}) == "foo");

  BOOST_TEST(yk::get<0>(yk::variant_view<Variant, int, double>{Variant{42}}) == 42);
  BOOST_TEST(yk::get<1>(yk::variant_view<Variant, int, double>{Variant{3.14}}) == 3.14);

  BOOST_TEST(yk::get<0>(yk::variant_view<const Variant, int, double>{Variant{42}}) == 42);
  BOOST_TEST(yk::get<1>(yk::variant_view<const Variant, int, double>{Variant{3.14}}) == 3.14);

  BOOST_TEST(yk::get<1>(yk::variant_view<Variant, double, int>{Variant{42}}) == 42);
  BOOST_TEST(yk::get<0>(yk::variant_view<Variant, double, int>{Variant{3.14}}) == 3.14);

  BOOST_TEST(yk::get<1>(yk::variant_view<const Variant, double, int>{Variant{42}}) == 42);
  BOOST_TEST(yk::get<0>(yk::variant_view<const Variant, double, int>{Variant{3.14}}) == 3.14);

  BOOST_TEST(yk::get<0>(yk::make_variant_view(Variant{3.14}).template subview<double, std::string>()) == 3.14);
  BOOST_TEST(yk::get<1>(yk::make_variant_view(Variant{"foo"}).template subview<double, std::string>()) == "foo");

  BOOST_REQUIRE_THROW(boost::ignore_unused(yk::get<double>(Variant{42})), std::bad_variant_access);
  BOOST_REQUIRE_THROW(boost::ignore_unused(yk::get<1>(Variant{42})), std::bad_variant_access);

  BOOST_REQUIRE_THROW(boost::ignore_unused(yk::get<double>(yk::make_variant_view(Variant{42}))), std::bad_variant_access);
  BOOST_REQUIRE_THROW(boost::ignore_unused(yk::get<1>(yk::make_variant_view(Variant{42}))), std::bad_variant_access);

  BOOST_REQUIRE_THROW(boost::ignore_unused(yk::get<double>(yk::make_variant_view(Variant{42}).template subview<double, std::string>())),
                      std::bad_variant_access);
  BOOST_REQUIRE_THROW(boost::ignore_unused(yk::get<1>(yk::make_variant_view(Variant{42}).template subview<double, std::string>())), std::bad_variant_access);

  {
    Variant var = 42;
    BOOST_TEST(yk::get<int>(&var) != nullptr);
    BOOST_TEST(yk::get<double>(&var) == nullptr);

    BOOST_TEST(yk::get<int>(&std::as_const(var)) != nullptr);
    BOOST_TEST(yk::get<double>(&std::as_const(var)) == nullptr);

    BOOST_TEST(yk::get<0>(&var) != nullptr);
    BOOST_TEST(yk::get<1>(&var) == nullptr);

    BOOST_TEST(yk::get<0>(&std::as_const(var)) != nullptr);
    BOOST_TEST(yk::get<1>(&std::as_const(var)) == nullptr);

    auto const_view = yk::variant_view<const Variant, double, int>(var);
    auto mutable_view = yk::variant_view<Variant, double, int>(var);

    static_assert(std::is_same_v<const int*, decltype(yk::get<int>(&const_view))>);
    static_assert(std::is_same_v<int*, decltype(yk::get<int>(&mutable_view))>);

    static_assert(std::is_same_v<const int*, decltype(yk::get<int>(&std::as_const(const_view)))>);
    static_assert(std::is_same_v<int*, decltype(yk::get<int>(&std::as_const(mutable_view)))>);

    BOOST_TEST(yk::get<int>(&const_view) != nullptr);
    BOOST_TEST(yk::get<double>(&const_view) == nullptr);

    BOOST_TEST(yk::get<int>(&std::as_const(const_view)) != nullptr);
    BOOST_TEST(yk::get<double>(&std::as_const(const_view)) == nullptr);

    BOOST_TEST(yk::get<0>(&const_view) == nullptr);
    BOOST_TEST(yk::get<1>(&const_view) != nullptr);

    BOOST_TEST(yk::get<0>(&std::as_const(const_view)) == nullptr);
    BOOST_TEST(yk::get<1>(&std::as_const(const_view)) != nullptr);
  }
}

struct S {
  int member;
};

BOOST_AUTO_TEST_CASE_TEMPLATE(SimpleGet, Variant, YK_VARIANT(int, double, std::string, S)) {
  BOOST_REQUIRE_THROW((boost::ignore_unused(*yk::variant_view<const Variant, int>{})), yk::uninitialized_variant_view);
  BOOST_REQUIRE_THROW((boost::ignore_unused(*yk::variant_view<Variant, int>{})), yk::uninitialized_variant_view);

  BOOST_REQUIRE_THROW(boost::ignore_unused(yk::variant_view<const Variant, S> {} -> member), yk::uninitialized_variant_view);
  BOOST_REQUIRE_THROW(boost::ignore_unused(yk::variant_view<Variant, S> {} -> member), yk::uninitialized_variant_view);

  BOOST_TEST(!static_cast<bool>(yk::variant_view<const Variant, int>{}));
  BOOST_TEST(!static_cast<bool>(yk::variant_view<Variant, int>{}));

  {
    Variant var = 42;
    auto const_view = yk::variant_view<const Variant, int>(var);
    auto mutable_view = yk::variant_view<Variant, int>(var);

    static_assert(std::is_same_v<const int&, decltype(*const_view)>);
    static_assert(std::is_same_v<int&, decltype(*mutable_view)>);

    BOOST_TEST((*const_view == 42));
    BOOST_TEST((*mutable_view == 42));

    var = 3.14;

    BOOST_REQUIRE_THROW(boost::ignore_unused(*const_view), std::bad_variant_access);
    BOOST_REQUIRE_THROW(boost::ignore_unused(*mutable_view), std::bad_variant_access);
  }
  {
    Variant var = 42;
    auto const_view = yk::variant_view<const Variant, S>(var);
    auto mutable_view = yk::variant_view<Variant, S>(var);

    BOOST_TEST(!static_cast<bool>(const_view));

    BOOST_REQUIRE_THROW(boost::ignore_unused(const_view->member), std::bad_variant_access);
    BOOST_REQUIRE_THROW(boost::ignore_unused(mutable_view->member), std::bad_variant_access);

    var = S{42};
    BOOST_TEST(static_cast<bool>(const_view));

    static_assert(std::is_same_v<const S*, decltype(const_view.operator->())>);
    static_assert(std::is_same_v<S*, decltype(mutable_view.operator->())>);

    BOOST_TEST((const_view->member == 42));
    BOOST_TEST((mutable_view->member == 42));
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(ComparisonOperator, Variant, YK_VARIANT(int, double, std::string)) {
  Variant a = 12, b = 3.14;
  auto a_view = yk::make_variant_view<int, double>(a);
  auto b_view = yk::make_variant_view<int, double>(b);

  BOOST_TEST((a == a_view));
  BOOST_TEST((a != b_view));

  BOOST_TEST((a_view == a_view));
  BOOST_TEST((a_view != b_view));

  BOOST_TEST(((a <=> b_view) < 0));
  BOOST_TEST(((a <=> a_view) == 0));
  BOOST_TEST(((b <=> a_view) > 0));

  BOOST_TEST(((a_view <=> b_view) < 0));
  BOOST_TEST(((a_view <=> a_view) == 0));
  BOOST_TEST(((b_view <=> a_view) > 0));

  BOOST_TEST((yk::compare_three_way{}(a, b) == yk::compare_three_way{}(a_view, b_view)));

  BOOST_TEST((yk::variant_view<Variant, int, double>{} == yk::variant_view<Variant, int, double>{}));
  BOOST_TEST((yk::variant_view<Variant, int, double>{} != a_view));

  BOOST_TEST(((yk::variant_view<Variant, int, double>{} <=> yk::variant_view<Variant, int, double>{}) == 0));
  BOOST_TEST(((yk::variant_view<Variant, int, double>{} <=> a_view) < 0));

  a = "foo";
  b = "bar";

  BOOST_TEST((a_view != b_view));

  BOOST_TEST(((a_view <=> b_view) > 0));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Swap, Variant, YK_VARIANT(int, double, std::string)) {
  Variant a = 33, b = 4;
  auto aa = yk::make_variant_view<int>(a);
  auto bb = yk::make_variant_view<int>(b);
  BOOST_TEST((*aa == 33 && *bb == 4));
  aa.swap(bb);
  BOOST_TEST((*aa == 4 && *bb == 33));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Hash, Variant, YK_VARIANT(int, double, std::string)) {
  Variant a = 42, b = 42, c = 3.14;
  std::unordered_set<yk::variant_view<Variant, int, double>> set{
      yk::variant_view<Variant, int, double>{a},
      yk::variant_view<Variant, int, double>{b},
      yk::variant_view<Variant, int, double>{c},
  };
  BOOST_TEST(set.size() == 2);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Index, Variant, YK_VARIANT(int, double, std::string)) {
  Variant a = 33, b = 4;
  if constexpr (yk::is_specialization_of_v<Variant, std::variant>) {
    BOOST_TEST(yk::make_variant_view<int>(a).index() == a.index());
  } else {
    BOOST_TEST(yk::make_variant_view<int>(a).index() == static_cast<std::size_t>(a.which()));
  }
  BOOST_TEST(yk::make_variant_view<int>(a).index() == yk::make_variant_view<int>(b).index());
}

BOOST_AUTO_TEST_CASE(MultiVisit) {
  std::variant<int, double, std::string> stdVariant = 42;
  boost::variant<int, double, std::string> boostVariant = 3.14;

  yk::visit(
      [](auto&& x, auto&& y) {
        BOOST_TEST((typeid(decltype(x)) == typeid(int)));
        BOOST_TEST((typeid(decltype(y)) == typeid(double)));
      },
      stdVariant, boostVariant);

  yk::visit(
      [](auto&& x, auto&& y) mutable {
        BOOST_TEST((typeid(decltype(x)) == typeid(int)));
        BOOST_TEST((typeid(decltype(y)) == typeid(double)));
      },
      stdVariant, boostVariant);

  yk::visit<void>(
      [](auto&& x, auto&& y) {
        BOOST_TEST((typeid(decltype(x)) == typeid(int)));
        BOOST_TEST((typeid(decltype(y)) == typeid(double)));
      },
      stdVariant, boostVariant);

  yk::visit<void>(
      [](auto&& x, auto&& y) mutable {
        BOOST_TEST((typeid(decltype(x)) == typeid(int)));
        BOOST_TEST((typeid(decltype(y)) == typeid(double)));
      },
      stdVariant, boostVariant);
}

BOOST_AUTO_TEST_SUITE_END()  // variant_view
