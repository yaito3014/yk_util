#ifndef YK_VARIANT_VIEW_VARIANT_VIEW_HPP
#define YK_VARIANT_VIEW_VARIANT_VIEW_HPP

#include "yk/util/pack_indexing.hpp"
#include "yk/lifetimebound.hpp"

#include "yk/variant.hpp"
#include "yk/variant/traits.hpp"

#include "yk/variant_view/traits.hpp"

#include <compare>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <cassert>
#include <cstddef>

namespace yk {

namespace detail {

template <class VariantView, class... Ts>
struct are_all_in_variant_view : std::conjunction<is_in_variant_view<VariantView, Ts>...> {};

template <class VariantView, class... Ts>
inline constexpr bool are_all_in_variant_view_v = are_all_in_variant_view<VariantView, Ts...>::value;

template <class Variant>
struct compare_impl {
  using category_t = decltype(std::declval<Variant>() <=> std::declval<Variant>());
  static constexpr auto apply(const Variant& lhs, const Variant& rhs) { return lhs <=> rhs; }
};

}  // namespace detail

class uninitialized_variant_view : public std::logic_error {
public:
  uninitialized_variant_view() : logic_error("accessing empty variant_view") {}
};

template <class Variant, class... Ts>
class variant_view {
public:
  static_assert(sizeof...(Ts) > 0, "empty variant_view is not allowed");
  static_assert(is_variant_like_v<std::remove_cvref_t<Variant>>, "argument should be variant-like type");
  static_assert((... && is_in_variant_v<std::remove_cvref_t<Variant>, Ts>), "variant_view must be subset of original variant");

  using variant_type = std::remove_const_t<Variant>;

  constexpr variant_view() noexcept : base_(nullptr) {}

  constexpr variant_view(const variant_view&) noexcept = default;
  constexpr variant_view(variant_view&&) noexcept = default;

  explicit constexpr variant_view(Variant& variant YK_LIFETIMEBOUND) noexcept : base_(&variant) {}

  explicit constexpr variant_view(variant_type&& variant YK_LIFETIMEBOUND) noexcept : base_(&variant) {}

  template <class V, class... Us>
    requires(std::is_const_v<Variant> || !std::is_const_v<V>)
  constexpr variant_view(const variant_view<V, Us...>& other) noexcept : base_(other.base_) {
    static_assert(detail::are_all_in_variant_view_v<variant_view<std::remove_const_t<Variant>, Us...>, Us...>, "only operations which take subset is allowed");
  }

  constexpr variant_view& operator=(const variant_view&) noexcept = default;
  constexpr variant_view& operator=(variant_view&&) noexcept = default;

  template <class... Us>
  constexpr variant_view& operator=(const variant_view<std::remove_const_t<Variant>, Us...>& other) noexcept {
    base_ = other.base_;
    return *this;
  }

  [[nodiscard]] constexpr const variant_type& base() const noexcept YK_LIFETIMEBOUND {
    assert(base_ != nullptr);
    return *base_;
  }
  [[nodiscard]] constexpr variant_type& base() const noexcept YK_LIFETIMEBOUND
    requires(!std::is_const_v<Variant>)
  {
    assert(base_ != nullptr);
    return *base_;
  }

  [[nodiscard]] constexpr decltype(auto) operator*() const YK_LIFETIMEBOUND
    requires(sizeof...(Ts) == 1);

  [[nodiscard]] constexpr auto operator->() const YK_LIFETIMEBOUND
    requires(sizeof...(Ts) == 1);

  [[nodiscard]] explicit constexpr operator bool() const noexcept
    requires(sizeof...(Ts) == 1);

  template <class... Us>
  [[nodiscard]] constexpr variant_view<Variant, Us...> subview() const noexcept {
    return variant_view<Variant, Us...>{*this};
  }

  template <class Visitor>
  constexpr decltype(auto) visit(Visitor&& vis) const;

  template <class Res, class Visitor>
  constexpr Res visit(Visitor&& vis) const;

  [[nodiscard]] constexpr std::size_t index() const noexcept { return variant_dispatch<std::remove_const_t<Variant>>::apply_index(base()); }
  [[nodiscard]] constexpr bool invalid() const noexcept { return base_ == nullptr; }

  template <class V>
    requires std::same_as<std::remove_const_t<Variant>, std::remove_const_t<V>>
  [[nodiscard]] constexpr bool operator==(const variant_view<V, Ts...>& other) const noexcept {
    if (invalid()) {
      if (other.invalid())
        return true;
      else
        return false;
    }
    if (other.invalid()) return false;
    return base() == other.base();
  }

  template <class V>
    requires std::same_as<std::remove_const_t<Variant>, std::remove_const_t<V>>
  [[nodiscard]] constexpr auto operator<=>(const variant_view<V, Ts...>& other) const noexcept {
    return [&]() -> typename detail::compare_impl<std::remove_const_t<Variant>>::category_t {
      if (invalid()) {
        if (other.invalid())
          return std::strong_ordering::equivalent;
        else
          return std::strong_ordering::less;
      }
      if (other.invalid()) return std::strong_ordering::greater;
      return detail::compare_impl<std::remove_const_t<Variant>>::apply(base(), other.base());
    }();
  }

  [[nodiscard]] constexpr bool operator==(const Variant& other) const noexcept { return *this == variant_view<const Variant, Ts...>{other}; }
  [[nodiscard]] constexpr auto operator<=>(const Variant& other) const noexcept { return *this <=> variant_view<const Variant, Ts...>{other}; }

  constexpr void swap(variant_view& other) noexcept { std::swap(base_, other.base_); }

private:
  template <class V, class... Us>
  friend class variant_view;

  Variant* base_;
};

template <class... Ts, class Variant>
[[nodiscard]] constexpr auto make_variant_view(Variant&& variant YK_LIFETIMEBOUND) noexcept {
  return make_variant_view_result_t<std::remove_reference_t<Variant>, Ts...>{std::forward<Variant>(variant)};
}

template <class Variant, class... Ts>
template <class Visitor>
constexpr decltype(auto) variant_view<Variant, Ts...>::visit(Visitor&& vis) const {
  return yk::visit(std::forward<Visitor>(vis), *this);
}

template <class Variant, class... Ts>
template <class Res, class Visitor>
constexpr Res variant_view<Variant, Ts...>::visit(Visitor&& vis) const {
  return yk::visit<Res>(std::forward<Visitor>(vis), *this);
}

template <class Variant, class... Ts>
[[nodiscard]] constexpr decltype(auto) variant_view<Variant, Ts...>::operator*() const
  requires(sizeof...(Ts) == 1)
{
  if (invalid()) throw uninitialized_variant_view{};
  return yk::get<0>(*this);
}

template <class Variant, class... Ts>
[[nodiscard]] constexpr auto variant_view<Variant, Ts...>::operator->() const
  requires(sizeof...(Ts) == 1)
{
  if (invalid()) throw uninitialized_variant_view{};
  auto res = yk::get<0>(this);
  if (res == nullptr) throw std::bad_variant_access{};
  return res;
}

template <class Variant, class... Ts>
[[nodiscard]] constexpr variant_view<Variant, Ts...>::operator bool() const noexcept
  requires(sizeof...(Ts) == 1)
{
  return base_ != nullptr && yk::get<0>(this) != nullptr;
}

template <class Variant, class... Ts>
constexpr void swap(variant_view<Variant, Ts...>& x, variant_view<Variant, Ts...>& y) noexcept {
  x.swap(y);
}

namespace detail {

template <class Visitor, class Variant, class... Ts>
struct SupersetTypeCatcher {
  using deduced_return_type = std::invoke_result_t<Visitor, pack_indexing_t<0, Ts...>>;

  Visitor vis;
  template <class T>
  constexpr deduced_return_type operator()(T&& x) const {
    return std::invoke(std::forward<Visitor>(vis), std::forward<T>(x));
  }

  template <class T>
  constexpr deduced_return_type operator()(T&& x) {
    return std::invoke(std::forward<Visitor>(vis), std::forward<T>(x));
  }

  template <class T>
    requires(!is_in_variant_view_v<variant_view<std::remove_const_t<Variant>, Ts...>, std::remove_cvref_t<T>>)
  [[noreturn]] constexpr deduced_return_type operator()(T&&) const {
    throw std::bad_variant_access{};
  }

  template <class T>
    requires(!is_in_variant_view_v<variant_view<std::remove_const_t<Variant>, Ts...>, std::remove_cvref_t<T>>)
  [[noreturn]] constexpr deduced_return_type operator()(T&&) {
    throw std::bad_variant_access{};
  }
};

}  // namespace detail

template <class Variant, class... Ts>
struct is_variant_like<variant_view<Variant, Ts...>> : std::true_type {};

template <class Variant, class... Ts>
struct variant_dispatch<variant_view<Variant, Ts...>> {
  template <class Visitor, class V>
  static constexpr decltype(auto) apply_visit(Visitor&& vis, V&& view) {
    if (view.invalid()) throw uninitialized_variant_view{};
    return yk::visit(detail::SupersetTypeCatcher<Visitor, Variant, Ts...>{std::forward<Visitor>(vis)}, view.base());
  }

  template <class Res, class Visitor, class V>
  static constexpr Res apply_visit(Visitor&& vis, V&& view) {
    if (view.invalid()) throw uninitialized_variant_view{};
    return yk::visit<Res>(detail::SupersetTypeCatcher<Visitor, Variant, Ts...>{std::forward<Visitor>(vis)}, view.base());
  }

  template <class T, class VariantView>
    requires(!std::is_pointer_v<std::remove_cvref_t<VariantView>>)
  static constexpr decltype(auto) apply_get(VariantView&& view) {
    if (view.invalid()) throw uninitialized_variant_view{};
    return yk::get<T>(std::forward<VariantView>(view).base());
  }

  template <std::size_t I>
  static constexpr decltype(auto) apply_get(const variant_view<Variant, Ts...>& view) {
    if (view.invalid()) throw uninitialized_variant_view{};
    return yk::get<pack_indexing_t<I, Ts...>>(view.base());
  }
  template <std::size_t I>
  static constexpr decltype(auto) apply_get(variant_view<Variant, Ts...>& view) {
    if (view.invalid()) throw uninitialized_variant_view{};
    return yk::get<pack_indexing_t<I, Ts...>>(view.base());
  }
  template <std::size_t I>
  static constexpr decltype(auto) apply_get(variant_view<Variant, Ts...>&& view) {
    if (view.invalid()) throw uninitialized_variant_view{};
    return yk::get<pack_indexing_t<I, Ts...>>(std::move(view).base());
  }
  template <std::size_t I>
  static constexpr decltype(auto) apply_get(const variant_view<Variant, Ts...>&& view) {
    if (view.invalid()) throw uninitialized_variant_view{};
    return yk::get<pack_indexing_t<I, Ts...>>(std::move(view).base());
  }

  template <class T, class VariantView>
  static constexpr auto apply_get(const VariantView* view) noexcept {
    if (view == nullptr || view->invalid()) return static_cast<decltype(yk::get<T>(&view->base()))>(nullptr);
    return yk::get<T>(&view->base());
  }

  template <std::size_t I>
  static constexpr auto apply_get(const variant_view<Variant, Ts...>* view) noexcept {
    if (view == nullptr || view->invalid()) return static_cast<decltype(yk::get<pack_indexing_t<I, Ts...>>(&view->base()))>(nullptr);
    return yk::get<pack_indexing_t<I, Ts...>>(&view->base());
  }
};

}  // namespace yk

#endif  // YK_VARIANT_VIEW_VARIANT_VIEW_HPP
