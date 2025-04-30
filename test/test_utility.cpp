#include "test_utility.hpp"

#include "yk/throwt.hpp"


namespace yk::testing {

void throw_std_exception()
{
  throwt<std::exception>();
}

} // yk::testing
