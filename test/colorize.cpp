#include <yk/colorize.hpp>
#include <yk/print.hpp>

#include <boost/test/unit_test.hpp>

#include <sstream>

BOOST_AUTO_TEST_SUITE(print)

BOOST_AUTO_TEST_CASE(colored_format_string)
{
  constexpr auto test = [](yk::colored_format_string<>) {};
  test("foo");
  test("[black]");
  test("[red]");
  test("[green]");
  test("[yellow]");
  test("[blue]");
  test("[magenta]");
  test("[cyan]");
  test("[white]");
}


#if __cpp_lib_format >= 202311L

BOOST_AUTO_TEST_CASE(forward_to_std_print)
{
  std::stringstream ss;
  yk::print(ss, "{}", 42);
  BOOST_TEST(ss.str() == "42");
}

#endif

BOOST_AUTO_TEST_SUITE_END()
