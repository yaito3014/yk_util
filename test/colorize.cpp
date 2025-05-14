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

  {
    auto s = yk::colorize("[red]foo[green]bar[reset]baz");
    BOOST_TEST(s == "\033[0;31mfoo\033[0;32mbar\033[0mbaz");
  }
}

BOOST_AUTO_TEST_CASE(format_and_colorize)
{
  {
    std::string s;
    yk::format_and_colorize_to(std::back_inserter(s), "[red]{}", 42);
    BOOST_TEST(s == "\033[0;31m42");
  }

  {
    auto s = yk::format_and_colorize("[green]{}", 42);
    BOOST_TEST(s == "\033[0;32m42");
  }
}

BOOST_AUTO_TEST_CASE(print)
{
  {
    std::stringstream ss;
    yk::print(ss, "{}", 42);
    BOOST_TEST(ss.str() == "42");
  }

  {
    std::stringstream ss;
    yk::print(ss, "[yellow]{}", 42);
    BOOST_TEST(ss.str() == "\033[0;33m42");
  }
}

BOOST_AUTO_TEST_SUITE_END()
