#include "test_utility.hpp"

#include "yk/throwt.hpp"


namespace yk::testing {

void throw_std_exception()
{
  throwt<std::exception>();
}

void throw_runtime_error(const char* what)
{
  throwt<std::runtime_error>(what);
}

void throw_system_error(std::error_code ec, const char* what)
{
  throwt<std::system_error>(ec, what);
}

void throw_system_error(int ev, const std::error_category& ecat, const char* what)
{
  throwt<std::system_error>(ev, ecat, what);
}

} // yk::testing
