#include <yk/colorize.hpp>
#include <yk/print.hpp>

#include <boost/test/unit_test.hpp>

#include <sstream>

BOOST_AUTO_TEST_SUITE(print)

BOOST_AUTO_TEST_CASE(colorize_string)
{
  constexpr auto test = [](yk::colorize_string) {};
  test("foo");
  test("[reset]");
  test("[fg:reset]");
  test("[bg:reset]");
  test("[black]");
  test("[red]");
  test("[green]");
  test("[yellow]");
  test("[blue]");
  test("[magenta]");
  test("[cyan]");
  test("[white]");
  test("[red|bold]");
  test("[bold|italic]");
  test("[fg:red|bg:blue]");

  BOOST_CHECK_THROW(yk::colorize(yk::runtime_colorize("[]")), yk::colorize_error);
  BOOST_CHECK_THROW(yk::colorize(yk::runtime_colorize("[")), yk::colorize_error);
  BOOST_CHECK_THROW(yk::colorize(yk::runtime_colorize("]")), yk::colorize_error);
  BOOST_CHECK_THROW(yk::colorize(yk::runtime_colorize("[reset|red]")), yk::colorize_error);
  BOOST_CHECK_THROW(yk::colorize(yk::runtime_colorize("[red|reset]")), yk::colorize_error);
  BOOST_CHECK_THROW(yk::colorize(yk::runtime_colorize("[black|red]")), yk::colorize_error);
}

BOOST_AUTO_TEST_CASE(colorize_format_string)
{
  constexpr auto test = [](yk::colorize_format_string<>) {};
  test("foo");
  test("[reset]");
  test("[fg:reset]");
  test("[bg:reset]");
  test("[black]");
  test("[red]");
  test("[green]");
  test("[yellow]");
  test("[blue]");
  test("[magenta]");
  test("[cyan]");
  test("[white]");
  test("[red|bold]");
  test("[bold|italic]");
  test("[fg:red|bg:blue]");
}

BOOST_AUTO_TEST_CASE(colorize)
{
  {
    std::string s;
    yk::colorize_to(std::back_inserter(s), "[red]foo");
    BOOST_TEST(s == "\033[38;2;255;0;0mfoo");
  }

  {
    const auto s = yk::colorize("[green]bar");
    BOOST_TEST(s == "\033[38;2;0;128;0mbar");
  }

  {
    const auto s = yk::colorize("[red]foo[green]bar[reset]baz");
    BOOST_TEST(s == "\033[38;2;255;0;0mfoo\033[38;2;0;128;0mbar\033[0mbaz");
  }

  {
    const auto s = yk::colorize("[red][[[green]]]");
    BOOST_TEST(s == "\033[38;2;255;0;0m[\033[38;2;0;128;0m]");
  }

  {
    const auto s = yk::colorize("[red|bold]foo");
    BOOST_TEST(s == "\033[1;38;2;255;0;0mfoo");
  }

  {
    const auto s = yk::colorize("[bold|italic]foo");
    BOOST_TEST(s == "\033[1;3mfoo");
  }

  {
    const auto s = yk::colorize("[fg:red|bg:blue]foo");
    BOOST_TEST(s == "\033[38;2;255;0;0;48;2;0;0;255mfoo");
  }

  {
    const auto s = yk::colorize("[fg:red|bg:blue]foo[fg:reset]bar[bg:reset]baz");
    BOOST_TEST(s == "\033[38;2;255;0;0;48;2;0;0;255mfoo\033[39mbar\033[49mbaz");
  }

  {
    const auto s = yk::colorize("[lime]foo");
    BOOST_TEST(s == "\033[38;2;0;255;0mfoo");
  }

  {
    const auto s = yk::colorize("[bg:gray]foo");
    BOOST_TEST(s == "\033[48;2;128;128;128mfoo");
  }

  {
    const auto s = yk::colorize("[rgb(12,34,56)]foo");
    BOOST_TEST(s == "\033[38;2;12;34;56mfoo");
  }

  BOOST_CHECK_THROW(yk::colorize(yk::runtime_colorize("[rgb(256,34,56)]foo")), yk::colorize_error);
  BOOST_CHECK_THROW(yk::colorize(yk::runtime_colorize("[rgb(-1,34,56)]foo")), yk::colorize_error);

  {
    const auto s = yk::colorize("[fg:rgb(12,34,56)]foo");
    BOOST_TEST(s == "\033[38;2;12;34;56mfoo");
  }

  {
    const auto s = yk::colorize("[bg:rgb(12,34,56)]foo");
    BOOST_TEST(s == "\033[48;2;12;34;56mfoo");
  }
  {
    const auto s = yk::colorize("[fg:rgb(12,34,56)|bg:rgb(78,90,12)]foo");
    BOOST_TEST(s == "\033[38;2;12;34;56;48;2;78;90;12mfoo");
  }

  BOOST_CHECK_THROW(yk::colorize(yk::runtime_colorize("[fg:rgb(12,34,56)|fg:rgb(78,90,12)]foo")), yk::colorize_error);
  BOOST_CHECK_THROW(yk::colorize(yk::runtime_colorize("[bg:rgb(12,34,56)|bg:rgb(78,90,12)]foo")), yk::colorize_error);
}

BOOST_AUTO_TEST_CASE(format_colorize)
{
  {
    std::string s;
    yk::format_colorize_to(std::back_inserter(s), "[red]{}", 42);
    BOOST_TEST(s == "\033[38;2;255;0;0m42");
  }

  {
    const auto s = yk::format_colorize("[green]{}", 42);
    BOOST_TEST(s == "\033[38;2;0;128;0m42");
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
    BOOST_TEST(ss.str() == "\033[38;2;255;255;0m42");
  }

  {
    std::stringstream ss;
    yk::colorize_config cfg(ss);
    yk::print(ss, cfg, "[yellow]{}", 42);
    BOOST_TEST(ss.str() == "42");
  }
}

BOOST_AUTO_TEST_SUITE_END()
