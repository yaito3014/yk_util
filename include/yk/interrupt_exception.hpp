#ifndef YK_INTERRUPT_EXCEPTION_HPP
#define YK_INTERRUPT_EXCEPTION_HPP

#include <stdexcept>
// #include <system_error> // TODO: support construction from EINTR

namespace yk {

class interrupt_exception : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;

  interrupt_exception() : std::runtime_error("interrupt") {}
};

}  // namespace yk

#endif
