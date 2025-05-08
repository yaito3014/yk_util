#include "yk/util/forward_like.hpp"
#include "yk/util/functional.hpp"
#include "yk/util/pack_indexing.hpp"
#include "yk/util/specialization_of.hpp"

#include <boost/test/unit_test.hpp>

#include <string>
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

BOOST_AUTO_TEST_CASE(FunctionTraits)
{
  {
    using F = int(float, double);
    static_assert(yk::function_traits<F>::kind == yk::function_kind::function);
    static_assert(std::is_same_v<yk::function_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::function_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = int (*)(float, double);
    static_assert(yk::function_traits<F>::kind == yk::function_kind::function);
    static_assert(std::is_same_v<yk::function_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::function_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }
  {
    using F = int (&)(float, double);
    static_assert(yk::function_traits<F>::kind == yk::function_kind::function);
    static_assert(std::is_same_v<yk::function_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::function_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }

  {
    struct S {};
    using F = int (S::*)(float, double);
    static_assert(yk::function_traits<F>::kind == yk::function_kind::member_function);
    static_assert(std::is_same_v<yk::function_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::function_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }

  {
    const auto lambda = [](float, double) -> int { return 42; };
    using F = decltype(lambda);
    static_assert(yk::function_traits<F>::kind == yk::function_kind::function_object);
    static_assert(std::is_same_v<yk::function_traits<F>::return_type, int>);
    static_assert(std::is_same_v<yk::function_traits<F>::parameters, yk::type_list<float, double>>);
    static_assert(!yk::is_unary_function_v<F>);
    static_assert(yk::is_binary_function_v<F>);
  }

  {
    const auto generic_lambda = [](auto) {};
    using F = decltype(generic_lambda);
    static_assert(yk::function_traits<F>::kind == yk::function_kind::unknown);
    static_assert(yk::is_unary_function_v<F>);   // fallback
    static_assert(yk::is_binary_function_v<F>);  // fallback
  }
}

BOOST_AUTO_TEST_SUITE_END()
