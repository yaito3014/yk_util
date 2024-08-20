#ifndef YK_UTIL_TO_SUBRANGE_BOOST_HPP
#define YK_UTIL_TO_SUBRANGE_BOOST_HPP

#include <boost/range/iterator_range.hpp>

namespace yk {

template <class Iterator>
constexpr auto to_subrange(boost::iterator_range<Iterator> rng) noexcept -> std::ranges::subrange<Iterator> {
  return std::ranges::subrange(rng.begin(), rng.end());
}

}  // namespace yk

#endif  // YK_UTIL_TO_SUBRANGE_BOOST_HPP
