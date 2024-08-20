#ifndef YK_UTIL_TO_SUBRANGE_BOOST_HPP
#define YK_UTIL_TO_SUBRANGE_BOOST_HPP

#include <boost/range/iterator_range.hpp>

namespace yk {

template <class Iterator>
[[nodiscard]] constexpr auto to_subrange(const boost::iterator_range<Iterator>& rng) noexcept(noexcept(std::ranges::subrange(rng.begin(), rng.end())))
    -> std::ranges::subrange<Iterator> {
  return std::ranges::subrange(rng.begin(), rng.end());
}

}  // namespace yk

#endif  // YK_UTIL_TO_SUBRANGE_BOOST_HPP
