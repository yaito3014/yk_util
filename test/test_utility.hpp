#ifndef YK_TESTING_TEST_UTILITY_HPP
#define YK_TESTING_TEST_UTILITY_HPP

#include <system_error>

namespace yk::testing {

void throw_std_exception();

void throw_runtime_error(const char* what);

void throw_system_error(std::error_code ec, const char* what);

void throw_system_error(int ev, const std::error_category&, const char* what);

} // yk::testing

#endif
