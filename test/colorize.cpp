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
  test("[red|bold]");

  BOOST_REQUIRE_THROW(yk::colorize(yk::runtime_colorize("[]")), yk::colorize_error);
  BOOST_REQUIRE_THROW(yk::colorize(yk::runtime_colorize("[")), yk::colorize_error);
  BOOST_REQUIRE_THROW(yk::colorize(yk::runtime_colorize("]")), yk::colorize_error);
  BOOST_REQUIRE_THROW(yk::colorize(yk::runtime_colorize("[reset|red]")), yk::colorize_error);
  BOOST_REQUIRE_THROW(yk::colorize(yk::runtime_colorize("[red|reset]")), yk::colorize_error);
  BOOST_REQUIRE_THROW(yk::colorize(yk::runtime_colorize("[black|red]")), yk::colorize_error);
  BOOST_REQUIRE_THROW(yk::colorize(yk::runtime_colorize("[bold|underline]")), yk::colorize_error);
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
  test("[red|bold]");
}

BOOST_AUTO_TEST_CASE(colorize)
{
  {
    std::string s;
    yk::colorize_to(std::back_inserter(s), "[red]foo");
    BOOST_TEST(s == "\033[31mfoo");
  }

  {
    const auto s = yk::colorize("[green]bar");
    BOOST_TEST(s == "\033[32mbar");
  }

  {
    const auto s = yk::colorize("[red]foo[green]bar[reset]baz");
    BOOST_TEST(s == "\033[31mfoo\033[32mbar\033[0mbaz");
  }

  {
    const auto s = yk::colorize("[red][[[green]]]");
    BOOST_TEST(s == "\033[31m[\033[32m]");
  }

  {
    const auto s = yk::colorize("[red|bold]foo");
    BOOST_TEST(s == "\033[1;31mfoo");
  }

  {
    static_assert(yk::colorize("[red]") == "\033[31m");
  }
}

BOOST_AUTO_TEST_CASE(format_and_colorize)
{
  {
    std::string s;
    yk::format_and_colorize_to(std::back_inserter(s), "[red]{}", 42);
    BOOST_TEST(s == "\033[31m42");
  }

  {
    const auto s = yk::format_and_colorize("[green]{}", 42);
    BOOST_TEST(s == "\033[32m42");
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
    BOOST_TEST(ss.str() == "\033[33m42");
  }
}

BOOST_AUTO_TEST_SUITE_END()
