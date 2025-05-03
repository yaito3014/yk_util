#include "yk/exec/atomic_queue.hpp"

#include "yk/arch.hpp"

#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <cstddef>


BOOST_AUTO_TEST_SUITE(atomic_queue)

namespace {

template <std::size_t Align, std::size_t Size>
struct alignas(Align) Align_Size
{
  std::byte data[Size];
};

struct Empty {};

} // anon

BOOST_AUTO_TEST_CASE(layout)
{
  { [[maybe_unused]] yk::exec::atomic_queue<std::uint8_t> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<std::uint16_t> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<std::uint32_t> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<std::uint64_t> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<std::max_align_t> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<char> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<unsigned char> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<std::byte> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<float> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<double> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<long double> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<void*> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Empty> q(1); }

  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 1>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 2>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 3>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 4>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 5>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 7>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 8>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 9>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 15>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 16>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 17>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 31>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 32>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 33>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 63>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 64>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 65>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 127>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 128>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<1, 129>> q(1); }

  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 1>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 2>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 3>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 4>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 5>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 7>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 8>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 9>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 15>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 16>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 17>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 31>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 32>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 33>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 63>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 64>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 65>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 127>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 128>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<2, 129>> q(1); }

  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 1>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 2>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 3>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 4>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 5>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 7>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 8>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 9>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 15>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 16>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 17>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 31>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 32>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 33>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 63>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 64>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 65>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 127>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 128>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<4, 129>> q(1); }

  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 1>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 2>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 3>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 4>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 5>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 7>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 8>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 9>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 15>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 16>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 17>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 31>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 32>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 33>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 63>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 64>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 65>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 127>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 128>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<8, 129>> q(1); }

  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 1>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 2>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 3>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 4>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 5>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 7>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 8>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 9>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 15>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 16>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 17>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 31>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 32>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 33>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 63>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 64>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 65>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 127>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 128>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<16, 129>> q(1); }

  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 1>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 2>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 3>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 4>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 5>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 7>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 8>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 9>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 15>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 16>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 17>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 31>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 32>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 33>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 63>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 64>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 65>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 127>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 128>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<32, 129>> q(1); }

  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 1>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 2>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 3>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 4>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 5>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 7>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 8>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 9>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 15>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 16>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 17>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 31>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 32>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 33>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 63>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 64>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 65>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 127>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 128>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<64, 129>> q(1); }

  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 1>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 2>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 3>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 4>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 5>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 7>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 8>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 9>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 15>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 16>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 17>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 31>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 32>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 33>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 63>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 64>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 65>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 127>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 128>> q(1); }
  { [[maybe_unused]] yk::exec::atomic_queue<Align_Size<128, 129>> q(1); }

}

BOOST_AUTO_TEST_SUITE_END()
