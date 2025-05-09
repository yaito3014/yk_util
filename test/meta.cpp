#include "yk/util/forward_like.hpp"
#include "yk/util/functional.hpp"
#include "yk/util/pack_indexing.hpp"
#include "yk/util/perfect_forward_as_tuple.hpp"
#include "yk/util/specialization_of.hpp"

#include <boost/test/unit_test.hpp>

#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(__cpp_lib_forward_like) && !(defined(__clang__) && defined(__GLIBCXX__))
#define YK_UTIL_STD_HAS_FORWARD_LIKE 1
#else
#define YK_UTIL_STD_HAS_FORWARD_LIKE 0
#endif

BOOST_AUTO_TEST_SUITE(meta)

BOOST_AUTO_TEST_CASE(PackIndexing)
{
  static_assert(std::is_same_v<yk::pack_indexing_t<0, int, double, std::string>, int>);
  static_assert(std::is_same_v<yk::pack_indexing_t<1, int, double, std::string>, double>);
  static_assert(std::is_same_v<yk::pack_indexing_t<2, int, double, std::string>, std::string>);
}

BOOST_AUTO_TEST_CASE(SpecializationOf)
{
  static_assert(yk::specialization_of<std::string, std::basic_string>);
  static_assert(yk::specialization_of<std::vector<int>, std::vector>);

  static_assert(!yk::specialization_of<std::string, std::vector>);
  static_assert(!yk::specialization_of<std::vector<int>, std::basic_string>);
}

#if YK_UTIL_STD_HAS_FORWARD_LIKE
template <class T, class U>
using std_fwd_like_t = decltype(std::forward_like<T>(std::declval<U>()));
#endif

BOOST_AUTO_TEST_CASE(ForwardLike)
{
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

BOOST_AUTO_TEST_CASE(PerfectForwardAsTuple) {
  int variable = 42;
  const auto tuple = yk::perfect_forward_as_tuple(33, variable, 4);
  static_assert(std::is_same_v<decltype(tuple), const std::tuple<int, int&, int>>);
}

namespace {

[[maybe_unused]] int unary_normal_function(float) { return 42; }
[[maybe_unused]] int binary_normal_function(float, double) { return 42; }

struct S {
  int data_member;

  int unary_member_function(float) { return 42; }
  int unary_member_function_c(float) const { return 42; }
  int unary_member_function_v(float) volatile { return 42; }
  int unary_member_function_cv(float) const volatile { return 42; }
  int unary_member_function_n(float) noexcept { return 42; }
  int unary_member_function_cn(float) const noexcept { return 42; }
  int unary_member_function_vn(float) volatile noexcept { return 42; }
  int unary_member_function_cvn(float) const volatile noexcept { return 42; }

  int binary_member_function(float, double) { return 42; }
  int binary_member_function_c(float, double) const { return 42; }
  int binary_member_function_v(float, double) volatile { return 42; }
  int binary_member_function_cv(float, double) const volatile { return 42; }
  int binary_member_function_n(float, double) noexcept { return 42; }
  int binary_member_function_cn(float, double) const noexcept { return 42; }
  int binary_member_function_vn(float, double) volatile noexcept { return 42; }
  int binary_member_function_cvn(float, double) const volatile noexcept { return 42; }
};

}  // namespace

BOOST_AUTO_TEST_CASE(FunctionTraits)
{
  {
    using F = decltype(unary_normal_function);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&unary_normal_function);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }
  {
    using F = decltype((unary_normal_function));
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }

  {
    using F = decltype(binary_normal_function);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&binary_normal_function);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype((binary_normal_function));
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }

  {
    using F = decltype(&S::unary_member_function);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::unary_member_function_c);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::unary_member_function_v);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::unary_member_function_cv);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::unary_member_function_n);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::unary_member_function_cn);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::unary_member_function_vn);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::unary_member_function_cvn);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float>>);
    static_assert(yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }

  {
    using F = decltype(binary_normal_function);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&binary_normal_function);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype((binary_normal_function));
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::binary_member_function);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::binary_member_function_c);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::binary_member_function_v);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::binary_member_function_cv);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::binary_member_function_n);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::binary_member_function_cn);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::binary_member_function_vn);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = decltype(&S::binary_member_function_cvn);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::member_function);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }

  {
    using F = decltype(&S::data_member);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::data_member);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(!yk::is_binary_function_v<F>);
  }

  {
    const auto lambda = [](float, double) -> int { return 42; };
    using F = decltype(lambda);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::function_object);
    static_assert(std::is_same_v<yk::invocable_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::invocable_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }

  {
    const auto generic_lambda = [](auto) {};
    using F = decltype(generic_lambda);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::generic_function_object);
    static_assert(yk::is_unary_function_v<F>);   // fallback
    static_assert(yk::is_binary_function_v<F>);  // fallback
  }

  {
    const auto generic_lambda = [](auto, auto) {};
    using F = decltype(generic_lambda);
    static_assert(yk::invocable_traits<F>::kind == yk::invocable_kind::generic_function_object);
    static_assert(yk::is_unary_function_v<F>);   // fallback
    static_assert(yk::is_binary_function_v<F>);  // fallback
  }
}

BOOST_AUTO_TEST_SUITE_END()
