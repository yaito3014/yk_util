#include <yk/colorize.hpp>
#include <yk/print.hpp>

#include <boost/test/unit_test.hpp>

#include <sstream>

BOOST_AUTO_TEST_SUITE(print)

BOOST_AUTO_TEST_CASE(colorize_string)
{
  constexpr auto test = [](yk::colorize_string) {};
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

BOOST_AUTO_TEST_CASE(colorize_format_string)
{
  constexpr auto test = [](yk::colorize_format_string<>) {};
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

BOOST_AUTO_TEST_CASE(colorize)
{
  {
    std::string s;
    yk::colorize_to(std::back_inserter(s), "[red]foo");
    BOOST_TEST(s == "\033[0;31mfoo");
  }

  {
    auto s = yk::colorize("[green]bar");
    BOOST_TEST(s == "\033[0;32mbar");
  }
}

#if __cpp_lib_format >= 202311L

BOOST_AUTO_TEST_CASE(forward_to_std_print)
{
  std::stringstream ss;
  yk::print(ss, "{}", 42);
  BOOST_TEST(ss.str() == "42");
  std::format("{}", 42);
}

#endif

BOOST_AUTO_TEST_SUITE_END()
