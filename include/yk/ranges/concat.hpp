#ifndef YK_RANGES_CONCAT_HPP
#define YK_RANGES_CONCAT_HPP

#include "yk/util/pack_indexing.hpp"

#include <concepts>
#include <ranges>
#include <tuple>
#include <utility>
#include <variant>

namespace yk {

namespace ranges {

namespace detail {

template <class R, class... Rs>
struct all_but_last_is_common {
  static constexpr bool value = std::ranges::common_range<R> && all_but_last_is_common<Rs...>::value;
};

template <class R>
struct all_but_last_is_common<R> {
  static constexpr bool value = true;
};

template <class R, class... Rs>
struct all_but_first_is_sized {
  static constexpr bool value = (std::ranges::sized_range<Rs> && ...);
};

}  // namespace detail

namespace xo {  // exposition_only

template <bool Const, class T>
using maybe_const = std::conditional_t<Const, const T, T>;

template <class R>
concept simple_view = std::ranges::view<R> && std::ranges::range<const R> && std::same_as<std::ranges::iterator_t<R>, std::ranges::iterator_t<const R>> &&
                      std::same_as<std::ranges::sentinel_t<R>, std::ranges::sentinel_t<const R>>;

template <class F, class Tuple>
constexpr auto tuple_transform(F&& f, Tuple&& t) {
  return std::apply([&]<class... Ts>(Ts&&... elements) { return std::tuple<std::invoke_result_t<F&, Ts>...>(std::invoke(f, std::forward<Ts>(elements))...); },
                    std::forward<Tuple>(t));
}

template <bool Const, class... Views>
concept all_random_access = (std::ranges::random_access_range<maybe_const<Const, Views>> && ...);

template <bool Const, class... Views>
concept all_bidirectional = (std::ranges::bidirectional_range<maybe_const<Const, Views>> && ...);

template <bool Const, class... Views>
concept all_forward = (std::ranges::forward_range<maybe_const<Const, Views>> && ...);

template <class... Rs>
using concat_reference_t = std::common_reference_t<std::ranges::range_reference_t<Rs>...>;

template <class... Rs>
using concat_value_t = std::common_type_t<std::ranges::range_value_t<Rs>...>;

template <class... Rs>
using concat_rvalue_reference_t = std::common_reference_t<std::ranges::range_rvalue_reference_t<Rs>...>;

template <class Ref, class RRef, class It>
concept concat_indirectly_readable_impl = requires(const It it) {
  { *it } -> std::convertible_to<Ref>;
  { std::ranges::iter_move(it) } -> std::convertible_to<RRef>;
};

template <class... Rs>
concept concat_indirectly_readable =
    std::common_reference_with<concat_reference_t<Rs...>&&, concat_value_t<Rs...>&> &&
    std::common_reference_with<concat_reference_t<Rs...>&&, concat_rvalue_reference_t<Rs...>&&> &&
    std::common_reference_with<concat_rvalue_reference_t<Rs...>&&, const concat_value_t<Rs...>&> &&
    (concat_indirectly_readable_impl<concat_reference_t<Rs>, concat_rvalue_reference_t<Rs>, std::ranges::iterator_t<Rs>> && ...);

template <class... Rs>
concept concatable = requires {
  typename concat_reference_t<Rs...>;
  typename concat_value_t<Rs...>;
  typename concat_rvalue_reference_t<Rs...>;
} && concat_indirectly_readable<Rs...>;

template <bool Const, class... Rs>
concept concat_is_random_access = all_random_access<Const, Rs...> && detail::all_but_last_is_common<xo::maybe_const<Const, Rs>...>::value;

template <bool Const, class... Rs>
concept concat_is_bidirectional = all_bidirectional<Const, Rs...> && detail::all_but_last_is_common<xo::maybe_const<Const, Rs>...>::value;

}  // namespace xo

namespace detail {

template <std::size_t Offset, class IndexSeq>
struct offset_index_sequence_impl {};

template <std::size_t Offset, std::size_t... Indice>
struct offset_index_sequence_impl<Offset, std::index_sequence<Indice...>> {
  using type = std::index_sequence<Offset + Indice...>;
};

template <std::size_t Offset, class IndexSeq>
using offset_index_sequence = typename offset_index_sequence_impl<Offset, IndexSeq>::type;

// make index sequence of [I, J)
template <std::size_t I, std::size_t J>
using make_index_range = offset_index_sequence<I, std::make_index_sequence<J - I>>;

struct concat_view_iterator_difference_helper {
  template <std::size_t I, std::size_t J, class Iterator>
  constexpr typename Iterator::difference_type operator()(const Iterator& x, const Iterator& y) const {
    if constexpr (I > J) {
      const auto dy = std::ranges::distance(std::get<J>(y.iter_), std::ranges::end(std::get<J>(y.parent_->views_)));
      const auto dx = std::ranges::distance(std::ranges::begin(std::get<I>(x.parent_->views_)), std::get<I>(x.iter_));
      typename Iterator::difference_type s = [&]<std::size_t... Indice>(std::index_sequence<Indice...>) {
        return (0 + ... + std::ranges::size(std::get<Indice>(x.parent_->views_)));
      }(make_index_range<J + 1, I>{});
      return dy + s + dx;
    } else if constexpr (I < J) {
      return -(y - x);
    } else {
      return std::get<I>(x.iter_) - std::get<J>(y.iter_);
    }
  }

  template <std::size_t I, std::size_t ViewsCount, class Iterator>
  constexpr typename Iterator::difference_type operator()(const Iterator& x) const {
    const auto dx = std::ranges::distance(std::get<I>(x.iter_), std::ranges::end(std::get<I>(x.parent_->views_)));
    typename Iterator::difference_type s = [&]<std::size_t... Indice>(std::index_sequence<Indice...>) {
      return (0 + ... + std::ranges::size(std::get<Indice>(x.parent_->views_)));
    }(make_index_range<I + 1, ViewsCount>{});
    return -(dx + s);
  }
};

struct concat_view_iterator_equality_helper {
  template <std::size_t LastIdx, class Iterator>
  constexpr bool operator()(const Iterator& it) const {
    return it.iter_.index() == LastIdx && std::get<LastIdx>(it.iter_) == std::ranges::end(std::get<LastIdx>(it.parent_->views_));
  }
};

template <bool Const, class... Vs>
struct concat_view_iter_cat {};

template <bool Const, class... Vs>
  requires xo::all_forward<Const, Vs...>
struct concat_view_iter_cat<Const, Vs...> {
private:
  constexpr static auto iterator_category_impl() {
    if constexpr (std::is_reference_v<xo::concat_reference_t<xo::maybe_const<Const, Vs>...>>)
      return []<class... Cats>(Cats...) {
        if constexpr ((std::derived_from<Cats, std::random_access_iterator_tag> && ...) && xo::concat_is_random_access<Const, Vs...>)
          return std::random_access_iterator_tag{};
        else if constexpr ((std::derived_from<Cats, std::bidirectional_iterator_tag> && ...) && xo::concat_is_bidirectional<Const, Vs...>)
          return std::bidirectional_iterator_tag{};
        else if constexpr ((std::derived_from<Cats, std::forward_iterator_tag> && ...))
          return std::forward_iterator_tag{};
        else
          return std::input_iterator_tag{};
      }(typename std::iterator_traits<std::ranges::iterator_t<xo::maybe_const<Const, Vs>>>::iterator_category{}...);
    else
      return std::input_iterator_tag{};
  }

public:
  using iterator_category = decltype(iterator_category_impl());
};

}  // namespace detail

template <std::ranges::input_range... Views>
  requires (std::ranges::view<Views> && ...) && (sizeof...(Views) > 0) && xo::concatable<Views...>
class concat_view : public std::ranges::view_interface<concat_view<Views...>> {
private:
  template <bool Const>
    requires (std::ranges::view<Views> && ...) && (sizeof...(Views) > 0) && xo::concatable<Views...>
  class iterator : public detail::concat_view_iter_cat<Const, Views...> {
  private:
    using base_iter = std::variant<std::ranges::iterator_t<xo::maybe_const<Const, Views>>...>;

    constexpr static auto iterator_concept_impl() {
      if constexpr (xo::concat_is_random_access<Const, Views...>)
        return std::random_access_iterator_tag{};
      else if constexpr (xo::concat_is_bidirectional<Const, Views...>)
        return std::bidirectional_iterator_tag{};
      else if constexpr (xo::all_forward<Const, Views...>)
        return std::forward_iterator_tag{};
      else
        return std::input_iterator_tag{};
    }

    template <std::size_t Idx, class F>
    constexpr static auto invoke_with_runtime_index_impl(F&& f, std::size_t index) {
      if (Idx == index) {
        return f.template operator()<Idx>();
      }
      if constexpr (Idx + 1 < sizeof...(Views)) {
        return invoke_with_runtime_index_impl<Idx + 1>(std::forward<F>(f), index);
      } else {
        std::abort();
      }
    }

    template <class F>
    constexpr static auto invoke_with_runtime_index(F&& f, std::size_t index) {
      return invoke_with_runtime_index_impl<0>(std::forward<F>(f), index);
    }

    template <class F>
    constexpr auto invoke_with_runtime_index_member(F&& f) {
      return invoke_with_runtime_index(std::forward<F>(f), iter_.index());
    }

  public:
    // iterator_category defined in detail::concat_view_iter_cat
    using iterator_concept = decltype(iterator_concept_impl());
    using value_type = xo::concat_value_t<xo::maybe_const<Const, Views>...>;
    using difference_type = std::common_type_t<std::ranges::range_difference_t<xo::maybe_const<Const, Views>>...>;

    iterator() = default;

    constexpr iterator(iterator<!Const> it)
      requires Const && (std::convertible_to<std::ranges::iterator_t<Views>, std::ranges::iterator_t<const Views>> && ...)
        : parent_(it.parent_) {
      invoke_with_runtime_index_member([this, &it]<std::size_t Idx>() { iter_.template emplace<Idx>(std::get<Idx>(std::move(it.iter_))); });
    }

    [[nodiscard]] constexpr decltype(auto) operator*() const {
      using reference = xo::concat_reference_t<xo::maybe_const<Const, Views>...>;
      return std::visit([](auto&& it) -> reference { return *it; }, iter_);
    }

    constexpr iterator& operator++() {
      invoke_with_runtime_index_member([this]<std::size_t Idx>() {
        ++std::get<Idx>(iter_);
        satisfy<Idx>();
      });
      return *this;
    }
    constexpr void operator++(int) { ++*this; }

    constexpr iterator operator++(int)
      requires xo::all_forward<Const, Views...>
    {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator& operator--()
      requires xo::concat_is_bidirectional<Const, Views...>
    {
      invoke_with_runtime_index_member([this]<std::size_t Idx>() { prev<Idx>(); });
      return *this;
    }

    constexpr iterator operator--(int)
      requires xo::concat_is_bidirectional<Const, Views...>
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator& operator+=(difference_type n)
      requires xo::concat_is_random_access<Const, Views...>
    {
      invoke_with_runtime_index_member([this, n]<std::size_t Idx>() {
        if (n > 0) {
          advance_fwd<Idx>(std::get<Idx>(iter_) - std::ranges::begin(std::get<Idx>(parent_->views_)), n);
        } else if (n < 0) {
          advance_bwd<Idx>(std::get<Idx>(iter_) - std::ranges::begin(std::get<Idx>(parent_->views_)), -n);
        }
      });
      return *this;
    }

    constexpr iterator& operator-=(difference_type n)
      requires xo::concat_is_random_access<Const, Views...>
    {
      *this += -n;
      return *this;
    }

    [[nodiscard]] constexpr decltype(auto) operator[](difference_type n) const
      requires xo::concat_is_random_access<Const, Views...>
    {
      return *((*this) + n);
    }

    [[nodiscard]] friend constexpr bool operator==(const iterator& x, const iterator& y)
      requires (std::equality_comparable<std::ranges::iterator_t<xo::maybe_const<Const, Views>>> && ...)
    {
      return x.iter_ == y.iter_;
    }

    [[nodiscard]] friend constexpr bool operator==(const iterator& it, std::default_sentinel_t) {
      return detail::concat_view_iterator_equality_helper{}.operator()<sizeof...(Views) - 1>(it);
    }

    [[nodiscard]] friend constexpr bool operator<(const iterator& x, const iterator& y)
      requires xo::all_random_access<Const, Views...>
    {
      return x.iter_ < y.iter_;
    }

    [[nodiscard]] friend constexpr bool operator>(const iterator& x, const iterator& y)
      requires xo::all_random_access<Const, Views...>
    {
      return x.iter_ > y.iter_;
    }

    [[nodiscard]] friend constexpr bool operator<=(const iterator& x, const iterator& y)
      requires xo::all_random_access<Const, Views...>
    {
      return x.iter_ <= y.iter_;
    }

    [[nodiscard]] friend constexpr bool operator>=(const iterator& x, const iterator& y)
      requires xo::all_random_access<Const, Views...>
    {
      return x.iter_ >= y.iter_;
    }

    [[nodiscard]] friend constexpr auto operator<=>(const iterator& x, const iterator& y)
      requires (xo::all_random_access<Const, Views...> && (std::three_way_comparable<std::ranges::iterator_t<xo::maybe_const<Const, Views>>> && ...))
    {
      return x.iter_ <=> y.iter_;
    }

    [[nodiscard]] friend constexpr iterator operator+(const iterator& it, difference_type n)
      requires xo::concat_is_random_access<Const, Views...>
    {
      auto temp = it;
      temp += n;
      return temp;
    }

    [[nodiscard]] friend constexpr iterator operator+(difference_type n, const iterator& it)
      requires xo::concat_is_random_access<Const, Views...>
    {
      return it + n;
    }

    [[nodiscard]] friend constexpr iterator operator-(const iterator& it, difference_type n)
      requires xo::concat_is_random_access<Const, Views...>
    {
      auto temp = it;
      temp -= n;
      return temp;
    }

    [[nodiscard]] friend constexpr difference_type operator-(const iterator& x, const iterator& y)
      requires xo::concat_is_random_access<Const, Views...>
    {
      return iterator::invoke_with_runtime_index(
          [&]<std::size_t I>() -> difference_type {
            return iterator::invoke_with_runtime_index(
                [&]<std::size_t J>() -> difference_type { return detail::concat_view_iterator_difference_helper{}.operator()<I, J>(x, y); }, y.iter_.index());
          },
          x.iter_.index());
    }

    [[nodiscard]] friend constexpr difference_type operator-(const iterator& x, std::default_sentinel_t)
      requires (std::sized_sentinel_for<std::ranges::sentinel_t<xo::maybe_const<Const, Views>>, std::ranges::iterator_t<xo::maybe_const<Const, Views>>> &&
                ...) &&
               detail::all_but_first_is_sized<xo::maybe_const<Const, Views>...>::value
    {
      return iterator::invoke_with_runtime_index(
          [&]<std::size_t I>() { return detail::concat_view_iterator_difference_helper{}.operator()<I, sizeof...(Views)>(x); }, x.iter_.index());
    }

    [[nodiscard]] friend constexpr difference_type operator-(std::default_sentinel_t, const iterator& x)
      requires (std::sized_sentinel_for<std::ranges::sentinel_t<xo::maybe_const<Const, Views>>, std::ranges::iterator_t<xo::maybe_const<Const, Views>>> &&
                ...) &&
               detail::all_but_first_is_sized<xo::maybe_const<Const, Views>...>::value
    {
      return -(x - std::default_sentinel);
    }

    [[nodiscard]] friend constexpr decltype(auto) iter_move(const iterator& it) noexcept(
        ((std::is_nothrow_invocable_v<decltype(std::ranges::iter_move), const std::ranges::iterator_t<xo::maybe_const<Const, Views>>&> &&
          std::is_nothrow_convertible_v<std::ranges::range_rvalue_reference_t<xo::maybe_const<Const, Views>>,
                                        xo::concat_rvalue_reference_t<xo::maybe_const<Const, Views>>>) &&
         ...)) {
      return std::visit([](const auto& i) -> xo::concat_rvalue_reference_t<xo::maybe_const<Const, Views>...> { return std::ranges::iter_move(i); }, it.iter_);
    }

    friend constexpr void iter_swap(const iterator& x, const iterator& y) noexcept(
        (noexcept(std::ranges::swap(*x, *y)) && ... &&
         noexcept(std::ranges::iter_swap(std::declval<const std::ranges::iterator_t<xo::maybe_const<Const, Views>>>(),
                                         std::declval<const std::ranges::iterator_t<xo::maybe_const<Const, Views>>>()))))
      requires std::swappable_with<std::iter_reference_t<iterator>, std::iter_reference_t<iterator>> &&
               (... && std::indirectly_swappable<std::ranges::iterator_t<xo::maybe_const<Const, Views>>>)
    {
      std::visit(
          [&](const auto& it1, const auto& it2) {
            if constexpr (std::is_same_v<decltype(it1), decltype(it2)>) {
              std::ranges::iter_swap(it1, it2);
            } else {
              std::ranges::swap(*x, *y);
            }
          },
          x.iter_, y.iter_);
    }

  private:
    template <size_t N>
    constexpr void satisfy() {
      if constexpr (N < (sizeof...(Views) - 1)) {
        if (std::get<N>(iter_) == std::ranges::end(std::get<N>(parent_->views_))) {
          iter_.template emplace<N + 1>(std::ranges::begin(std::get<N + 1>(parent_->views_)));
          satisfy<N + 1>();
        }
      }
    }

    template <size_t N>
    constexpr void prev() {
      if constexpr (N == 0) {
        --std::get<0>(iter_);
      } else {
        if (std::get<N>(iter_) == std::ranges::begin(std::get<N>(parent_->views_))) {
          iter_.template emplace<N - 1>(std::ranges::end(std::get<N - 1>(parent_->views_)));
          prev<N - 1>();
        } else {
          --std::get<N>(iter_);
        }
      }
    }

    template <size_t N>
    constexpr void advance_fwd(difference_type offset, difference_type steps) {
      using underlying_diff_type = std::iter_difference_t<std::variant_alternative_t<N, base_iter>>;
      if constexpr (N == sizeof...(Views) - 1) {
        std::get<N>(iter_) += static_cast<underlying_diff_type>(steps);
      } else {
        auto n_size = std::ranges::distance(std::get<N>(parent_->views_));
        if (offset + steps < n_size) {
          std::get<N>(iter_) += static_cast<underlying_diff_type>(steps);
        } else {
          iter_.template emplace<N + 1>(std::ranges::begin(std::get<N + 1>(parent_->views_)));
          advance_fwd<N + 1>(0, offset + steps - n_size);
        }
      }
    }

    template <size_t N>
    constexpr void advance_bwd(difference_type offset, difference_type steps) {
      using underlying_diff_type = std::iter_difference_t<std::variant_alternative_t<N, base_iter>>;
      if constexpr (N == 0) {
        std::get<N>(iter_) -= static_cast<underlying_diff_type>(steps);
      } else {
        if (offset >= steps) {
          std::get<N>(iter_) -= static_cast<underlying_diff_type>(steps);
        } else {
          auto prev_size = std::ranges::distance(std::get<N - 1>(parent_->views_));
          iter_.template emplace<N - 1>(std::ranges::end(std::get<N - 1>(parent_->views_)));
          advance_bwd<N - 1>(prev_size, steps - offset);
        }
      }
    }

    template <class... Args>
    constexpr explicit iterator(xo::maybe_const<Const, concat_view<Views...>>* parent, Args&&... args)
      requires std::constructible_from<base_iter, Args...>
        : parent_(parent), iter_(std::forward<Args>(args)...) {}

    friend concat_view;
    friend iterator<!Const>;

    friend detail::concat_view_iterator_difference_helper;
    friend detail::concat_view_iterator_equality_helper;

  private:
    xo::maybe_const<Const, concat_view<Views...>>* parent_ = nullptr;
    base_iter iter_;
  };

public:
  constexpr concat_view() = default;
  constexpr explicit concat_view(Views... views) : views_(std::move(views)...) {}

  constexpr iterator<false> begin()
    requires (!(xo::simple_view<Views> && ...))
  {
    iterator<false> it(this, std::in_place_index<0>, std::ranges::begin(std::get<0>(views_)));
    it.template satisfy<0>();
    return it;
  }

  constexpr iterator<true> begin() const
    requires (std::ranges::range<const Views> && ...) && xo::concatable<const Views...>
  {
    iterator<true> it(this, std::in_place_index<0>, std::ranges::begin(std::get<0>(views_)));
    it.template satisfy<0>();
    return it;
  }

  constexpr auto end()
    requires (!(xo::simple_view<Views> && ...))
  {
    constexpr auto N = sizeof...(Views);
    // added semiregular constraint to correctly propagate common_range concept
    if constexpr ((std::semiregular<std::ranges::iterator_t<Views>> && ...) && std::ranges::common_range<pack_indexing_t<N - 1, Views...>>) {
      return iterator<false>(this, std::in_place_index<N - 1>, std::ranges::end(std::get<N - 1>(views_)));
    } else {
      return std::default_sentinel;
    }
  }
  constexpr auto end() const
    requires (std::ranges::range<const Views> && ...) && xo::concatable<const Views...>
  {
    constexpr auto N = sizeof...(Views);
    // added semiregular constraint to correctly propagate common_range concept
    if constexpr ((std::semiregular<std::ranges::iterator_t<const Views>> && ...) && std::ranges::common_range<const pack_indexing_t<N - 1, Views...>>) {
      return iterator<true>(this, std::in_place_index<N - 1>, std::ranges::end(std::get<N - 1>(views_)));
    } else {
      return std::default_sentinel;
    }
  }

  constexpr auto size()
    requires (std::ranges::sized_range<Views> && ...)
  {
    return std::apply(
        [](auto... sizes) {
          using CT = std::make_unsigned_t<std::common_type_t<decltype(sizes)...>>;
          return (CT(sizes) + ...);
        },
        xo::tuple_transform(std::ranges::size, views_));
  }
  constexpr auto size() const
    requires (std::ranges::sized_range<const Views> && ...)
  {
    return std::apply(
        [](auto... sizes) {
          using CT = std::make_unsigned_t<std::common_type_t<decltype(sizes)...>>;
          return (CT(sizes) + ...);
        },
        xo::tuple_transform(std::ranges::size, views_));
  }

private:
  friend detail::concat_view_iterator_difference_helper;
  friend detail::concat_view_iterator_equality_helper;

  std::tuple<Views...> views_;
};

template <class... Rs>
concat_view(Rs&&...) -> concat_view<std::views::all_t<Rs>...>;

namespace views {

struct concat_fn {
  template <class... Ts>
  [[nodiscard]] constexpr auto operator()(Ts&&... xs) const {
    if constexpr (sizeof...(Ts) == 1 && (std::ranges::input_range<Ts> && ...)) {
      return std::views::all(std::forward<Ts>(xs)...);
    } else {
      return concat_view(std::forward<Ts>(xs)...);
    }
  }
};

inline constexpr concat_fn concat;

}  // namespace views

}  // namespace ranges

namespace views = ranges::views;

}  // namespace yk

#endif  // YK_RANGES_CONCAT_HPP
