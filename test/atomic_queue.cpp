#include "yk/exec/atomic_queue.hpp"

#include "yk/allocator/default_init_allocator.hpp"

#include "yk/arch.hpp"

#include <boost/test/unit_test.hpp>

#include <memory>
#include <tuple>
#include <cstdint>
#include <cstddef>


namespace {

template <std::size_t Align, std::size_t Size>
struct alignas(Align) Align_Size
{
  std::byte data[Size];
};

struct Empty {};

using allocators_t = std::tuple<std::allocator<int>, yk::default_init_allocator<int>>;

template <class T, class Alloc>
using atomic_queue_t = yk::exec::atomic_queue<T, typename std::allocator_traits<Alloc>::template rebind_alloc<T>>;

template <class T, std::size_t N, class Alloc>
using static_atomic_queue_t = yk::exec::static_atomic_queue<T, N, typename std::allocator_traits<Alloc>::template rebind_alloc<T>>;

} // anon


BOOST_AUTO_TEST_SUITE(atomic_queue_layout)

BOOST_AUTO_TEST_CASE_TEMPLATE(dynamic_queue, Alloc, allocators_t)
{
  { [[maybe_unused]] atomic_queue_t<std::uint8_t, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<std::uint16_t, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<std::uint32_t, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<std::uint64_t, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<std::max_align_t, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<char, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<unsigned char, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<std::byte, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<float, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<double, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<long double, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<void*, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Empty, Alloc> q(1); }

  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 1>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 2>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 3>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 4>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 5>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 7>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 8>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 9>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 15>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 16>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 17>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 31>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 32>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 33>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 63>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 64>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 65>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 127>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 128>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<1, 129>, Alloc> q(1); }

  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 1>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 2>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 3>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 4>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 5>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 7>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 8>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 9>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 15>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 16>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 17>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 31>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 32>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 33>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 63>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 64>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 65>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 127>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 128>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<2, 129>, Alloc> q(1); }

  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 1>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 2>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 3>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 4>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 5>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 7>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 8>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 9>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 15>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 16>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 17>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 31>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 32>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 33>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 63>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 64>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 65>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 127>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 128>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<4, 129>, Alloc> q(1); }

  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 1>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 2>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 3>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 4>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 5>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 7>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 8>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 9>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 15>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 16>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 17>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 31>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 32>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 33>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 63>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 64>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 65>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 127>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 128>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<8, 129>, Alloc> q(1); }

  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 1>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 2>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 3>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 4>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 5>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 7>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 8>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 9>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 15>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 16>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 17>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 31>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 32>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 33>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 63>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 64>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 65>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 127>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 128>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<16, 129>, Alloc> q(1); }

  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 1>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 2>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 3>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 4>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 5>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 7>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 8>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 9>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 15>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 16>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 17>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 31>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 32>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 33>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 63>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 64>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 65>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 127>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 128>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<32, 129>, Alloc> q(1); }

  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 1>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 2>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 3>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 4>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 5>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 7>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 8>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 9>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 15>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 16>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 17>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 31>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 32>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 33>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 63>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 64>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 65>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 127>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 128>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<64, 129>, Alloc> q(1); }

  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 1>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 2>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 3>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 4>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 5>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 7>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 8>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 9>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 15>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 16>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 17>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 31>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 32>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 33>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 63>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 64>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 65>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 127>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 128>, Alloc> q(1); }
  { [[maybe_unused]] atomic_queue_t<Align_Size<128, 129>, Alloc> q(1); }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(static_queue, Alloc, allocators_t)
{
  { [[maybe_unused]] static_atomic_queue_t<std::uint8_t, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<std::uint16_t, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<std::uint32_t, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<std::uint64_t, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<std::max_align_t, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<char, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<unsigned char, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<std::byte, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<float, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<double, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<long double, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<void*, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Empty, 1, Alloc> q; }

  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 1>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 2>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 3>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 4>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 5>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 7>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 8>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 9>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 15>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 16>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 17>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 31>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 32>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 33>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 63>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 64>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 65>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 127>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 128>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<1, 129>, 1, Alloc> q; }

  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 1>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 2>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 3>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 4>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 5>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 7>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 8>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 9>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 15>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 16>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 17>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 31>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 32>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 33>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 63>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 64>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 65>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 127>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 128>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<2, 129>, 1, Alloc> q; }

  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 1>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 2>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 3>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 4>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 5>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 7>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 8>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 9>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 15>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 16>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 17>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 31>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 32>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 33>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 63>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 64>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 65>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 127>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 128>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<4, 129>, 1, Alloc> q; }

  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 1>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 2>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 3>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 4>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 5>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 7>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 8>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 9>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 15>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 16>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 17>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 31>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 32>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 33>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 63>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 64>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 65>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 127>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 128>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<8, 129>, 1, Alloc> q; }

  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 1>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 2>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 3>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 4>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 5>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 7>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 8>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 9>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 15>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 16>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 17>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 31>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 32>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 33>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 63>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 64>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 65>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 127>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 128>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<16, 129>, 1, Alloc> q; }

  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 1>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 2>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 3>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 4>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 5>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 7>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 8>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 9>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 15>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 16>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 17>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 31>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 32>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 33>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 63>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 64>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 65>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 127>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 128>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<32, 129>, 1, Alloc> q; }

  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 1>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 2>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 3>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 4>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 5>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 7>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 8>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 9>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 15>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 16>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 17>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 31>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 32>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 33>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 63>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 64>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 65>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 127>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 128>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<64, 129>, 1, Alloc> q; }

  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 1>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 2>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 3>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 4>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 5>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 7>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 8>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 9>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 15>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 16>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 17>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 31>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 32>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 33>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 63>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 64>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 65>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 127>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 128>, 1, Alloc> q; }
  { [[maybe_unused]] static_atomic_queue_t<Align_Size<128, 129>, 1, Alloc> q; }
}

BOOST_AUTO_TEST_SUITE_END() // atomic_queue_layout


BOOST_AUTO_TEST_SUITE(dynamic_atomic_queue)

BOOST_AUTO_TEST_CASE_TEMPLATE(traits, Alloc, allocators_t)
{
  using queue_type = atomic_queue_t<int, Alloc>;
  BOOST_REQUIRE_THROW(queue_type{0}, std::bad_alloc);
  BOOST_REQUIRE_NO_THROW(queue_type{1});
}

BOOST_AUTO_TEST_CASE_TEMPLATE(non_blocking, Alloc, allocators_t)
{
  // capacity = 1
  {
    atomic_queue_t<int, Alloc> q(1);
    BOOST_REQUIRE(q.capacity() == 1);
    BOOST_REQUIRE(q.size() == 0);

    int val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);

    BOOST_REQUIRE(q.try_push(42) == true);
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE(q.try_push(43) == false);
    BOOST_REQUIRE(q.size() == 1);

    BOOST_REQUIRE(q.try_pop(val) == true);
    BOOST_REQUIRE(val == 42);
    BOOST_REQUIRE(q.size() == 0);

    BOOST_REQUIRE(q.try_pop(val) == false);
  }

  // capacity = 2
  {
    atomic_queue_t<int, Alloc> q(2);
    BOOST_REQUIRE(q.capacity() == 2);
    BOOST_REQUIRE(q.size() == 0);

    int val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);

    BOOST_REQUIRE(q.try_push(42) == true);
    BOOST_REQUIRE(q.size() == 1);

    BOOST_REQUIRE(q.try_pop(val) == true);
    BOOST_REQUIRE(val == 42);
    BOOST_REQUIRE(q.size() == 0);

    BOOST_REQUIRE(q.try_pop(val) == false);

    // --------------------------------

    BOOST_REQUIRE(q.try_push(42) == true);
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE(q.try_push(43) == true);
    BOOST_REQUIRE(q.size() == 2);
    BOOST_REQUIRE(q.try_push(44) == false);
    BOOST_REQUIRE(q.size() == 2);

    val = 0;
    BOOST_REQUIRE(q.try_pop(val) == true);
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE(val == 42);

    val = 0;
    BOOST_REQUIRE(q.try_pop(val) == true);
    BOOST_REQUIRE(q.size() == 0);
    BOOST_REQUIRE(val == 43);

    val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(blocking, Alloc, allocators_t)
{
  // capacity = 1
  {
    atomic_queue_t<int, Alloc> q(1);
    BOOST_REQUIRE(q.capacity() == 1);
    BOOST_REQUIRE(q.size() == 0);

    int val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);

    BOOST_REQUIRE_NO_THROW(q.push(42));
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE(q.try_push(43) == false);
    BOOST_REQUIRE(q.size() == 1);

    BOOST_REQUIRE_NO_THROW(q.pop(val));
    BOOST_REQUIRE(val == 42);
    BOOST_REQUIRE(q.size() == 0);

    BOOST_REQUIRE(q.try_pop(val) == false);
  }

  // capacity = 2
  {
    atomic_queue_t<int, Alloc> q(2);
    BOOST_REQUIRE(q.capacity() == 2);
    BOOST_REQUIRE(q.size() == 0);

    int val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);

    BOOST_REQUIRE_NO_THROW(q.push(42));
    BOOST_REQUIRE(q.size() == 1);

    BOOST_REQUIRE_NO_THROW(q.pop(val));
    BOOST_REQUIRE(val == 42);
    BOOST_REQUIRE(q.size() == 0);

    BOOST_REQUIRE(q.try_pop(val) == false);

    // --------------------------------

    BOOST_REQUIRE_NO_THROW(q.push(42));
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE_NO_THROW(q.push(43));
    BOOST_REQUIRE(q.size() == 2);
    BOOST_REQUIRE(q.try_push(44) == false);
    BOOST_REQUIRE(q.size() == 2);

    val = 0;
    BOOST_REQUIRE_NO_THROW(q.pop(val));
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE(val == 42);

    val = 0;
    BOOST_REQUIRE_NO_THROW(q.pop(val));
    BOOST_REQUIRE(q.size() == 0);
    BOOST_REQUIRE(val == 43);

    val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);
  }
}

BOOST_AUTO_TEST_SUITE_END() // dynamic_atomic_queue


BOOST_AUTO_TEST_SUITE(static_atomic_queue, *boost::unit_test::disabled())

BOOST_AUTO_TEST_CASE_TEMPLATE(traits, Alloc, allocators_t)
{
  using queue_type = static_atomic_queue_t<int, 1, Alloc>;

  BOOST_REQUIRE_NO_THROW({
    [[maybe_unused]] queue_type q;
  });
  BOOST_REQUIRE_NO_THROW({
    [[maybe_unused]] queue_type q(1);
  });
  BOOST_REQUIRE_NO_THROW({
    [[maybe_unused]] queue_type q(0); // dynamic capacity lower than the static capacity shall be ignored by design
  });
  BOOST_REQUIRE_THROW({
    [[maybe_unused]] queue_type q(2);
  }, std::bad_alloc);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(non_blocking, Alloc, allocators_t)
{
  // capacity = 1
  {
    static_atomic_queue_t<int, 1, Alloc> q;
    BOOST_REQUIRE(q.capacity() == 1);
    BOOST_REQUIRE(q.size() == 0);

    int val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);

    BOOST_REQUIRE(q.try_push(42) == true);
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE(q.try_push(43) == false);
    BOOST_REQUIRE(q.size() == 1);

    BOOST_REQUIRE(q.try_pop(val) == true);
    BOOST_REQUIRE(val == 42);
    BOOST_REQUIRE(q.size() == 0);

    BOOST_REQUIRE(q.try_pop(val) == false);
  }

  // capacity = 2
  {
    static_atomic_queue_t<int, 2, Alloc> q;
    BOOST_REQUIRE(q.capacity() == 2);
    BOOST_REQUIRE(q.size() == 0);

    int val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);

    BOOST_REQUIRE(q.try_push(42) == true);
    BOOST_REQUIRE(q.size() == 1);

    BOOST_REQUIRE(q.try_pop(val) == true);
    BOOST_REQUIRE(val == 42);
    BOOST_REQUIRE(q.size() == 0);

    BOOST_REQUIRE(q.try_pop(val) == false);

    // --------------------------------

    BOOST_REQUIRE(q.try_push(42) == true);
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE(q.try_push(43) == true);
    BOOST_REQUIRE(q.size() == 2);
    BOOST_REQUIRE(q.try_push(44) == false);
    BOOST_REQUIRE(q.size() == 2);

    val = 0;
    BOOST_REQUIRE(q.try_pop(val) == true);
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE(val == 42);

    val = 0;
    BOOST_REQUIRE(q.try_pop(val) == true);
    BOOST_REQUIRE(q.size() == 0);
    BOOST_REQUIRE(val == 43);

    val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(blocking, Alloc, allocators_t)
{
  // capacity = 1
  {
    static_atomic_queue_t<int, 1, Alloc> q;
    BOOST_REQUIRE(q.capacity() == 1);
    BOOST_REQUIRE(q.size() == 0);

    int val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);

    BOOST_REQUIRE_NO_THROW(q.push(42));
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE(q.try_push(43) == false);
    BOOST_REQUIRE(q.size() == 1);

    BOOST_REQUIRE_NO_THROW(q.pop(val));
    BOOST_REQUIRE(val == 42);
    BOOST_REQUIRE(q.size() == 0);

    BOOST_REQUIRE(q.try_pop(val) == false);
  }

  // capacity = 2
  {
    static_atomic_queue_t<int, 2, Alloc> q;
    BOOST_REQUIRE(q.capacity() == 2);
    BOOST_REQUIRE(q.size() == 0);

    int val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);

    BOOST_REQUIRE_NO_THROW(q.push(42));
    BOOST_REQUIRE(q.size() == 1);

    BOOST_REQUIRE_NO_THROW(q.pop(val));
    BOOST_REQUIRE(val == 42);
    BOOST_REQUIRE(q.size() == 0);

    BOOST_REQUIRE(q.try_pop(val) == false);

    // --------------------------------

    BOOST_REQUIRE_NO_THROW(q.push(42));
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE_NO_THROW(q.push(43));
    BOOST_REQUIRE(q.size() == 2);
    BOOST_REQUIRE(q.try_push(44) == false);
    BOOST_REQUIRE(q.size() == 2);

    val = 0;
    BOOST_REQUIRE_NO_THROW(q.pop(val));
    BOOST_REQUIRE(q.size() == 1);
    BOOST_REQUIRE(val == 42);

    val = 0;
    BOOST_REQUIRE_NO_THROW(q.pop(val));
    BOOST_REQUIRE(q.size() == 0);
    BOOST_REQUIRE(val == 43);

    val = 0;
    BOOST_REQUIRE(q.try_pop(val) == false);
  }
}

BOOST_AUTO_TEST_SUITE_END() // static_atomic_queue
