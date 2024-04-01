/* Circle
 * STATUS
 *  tested: boj/7869,10216
 *  untested: boj/15004,7019,9598,20989
 */
#pragma once

#include <cmath>
#include <variant>

#include "geometry/point.h"

template <typename>
struct circle;

namespace geometry {
template <typename T>
struct circle_intersection_type {
  using type = T;
  using one = point<T>;
  using two = std::tuple<point<T>, point<T>>;
  std::variant<bool, one, two> it;
  auto count() const -> int {
    return std::holds_alternative<bool>(it) ? (std::get<bool>(it) ? -1 : 0)
                                            : (std::holds_alternative<one>(it) ? 1 : 2);
  }
  auto get_one() const -> point<T> { return std::get<one>(it); }
  auto get_two() const -> std::tuple<point<T>, point<T>> { return std::get<two>(it); }
};

template <typename T, typename U>
struct bigger_intersection_type {
  using T_inter = point<T>::intersection_t;
  using U_inter = point<U>::intersection_t;
  using type = std::conditional_t<sizeof(T_inter) < sizeof(U_inter), U_inter, T_inter>;
};

template <typename T, typename U>
using bigger_intersection_t = typename bigger_intersection_type<T, U>::type;

template <typename T, typename U>
using circle_intersection_t = circle_intersection_type<bigger_intersection_t<T, U>>;
}  // namespace geometry

template <typename T>
struct circle {
  using type = T;
  using product_t = point<T>::product_t;
  template <typename U>
  using intersection_t = geometry::circle_intersection_t<T, U>;

  point<T> center;
  T radius;

  circle(point<T> const& c, T const& r) : center(c), radius(r) {}
  circle(T x, T y, T r) : center(x, y), radius(r) {}
  template <typename U>
    requires(geometry::implicit_conversion_v<T, U>)
  circle(circle<U> const& c) : center(c.center), radius(c.radius) {}
  template <typename U>
    requires(not geometry::implicit_conversion_v<T, U>)
  explicit circle(circle<U> const& c) : center(c.center), radius(c.radius) {}

  bool operator==(const circle& o) const { return center == o.center and radius == o.radius; }

  auto area() const { return std::numbers::pi * radius * radius; }
  auto arc_length(std::floating_point auto angle) const { return angle * radius; }
  auto sector_area(std::floating_point auto angle) const { return angle * radius * radius / 2; }
  auto segment_area(std::floating_point auto angle) const {
    return (angle - std::sin(angle)) * radius * radius / 2;
  }

  template <std::floating_point U>
  auto get_point(U angle) const {
    return point<U>(center) + point<U>::polar(radius, angle);
  }

  template <std::floating_point F = T>
  bool contains(epsilon<F> eps, point<F> const& o, strict strict = {false}) const {
    if (strict) {
      return norm(o - center) < product_t(radius) * (radius - 2 * eps);
    } else {
      return norm(o - center) <= product_t(radius) * (radius + 2 * eps);
    }
  }
  bool contains(point<T> const& o, strict strict = {false}) const
    requires(geometry::non_floating<T>)
  {
    if (strict) {
      return norm(o - center) < product_t(radius) * radius;
    } else {
      return norm(o - center) <= product_t(radius) * radius;
    }
  }

  /// Points are ordered in ccw order around the first circle (`this`)
  /// ie. returns [before, after] intersection in the case of two intersections
  template <typename U>
  intersection_t<U> intersect(circle<U> const& o) const {
    using I = geometry::bigger_intersection_t<T, U>;
    if constexpr (not std::is_same_v<I, T> or not std::is_same_v<I, U>) {
      return circle<I>(*this).intersect(circle<I>(o));
    } else {
      return _intersect_impl(o);
    }
  }

  template <typename U, std::floating_point E>
  intersection_t<U> intersect(circle<U> const& o, epsilon<E> eps) const {
    using I = geometry::bigger_intersection_t<T, U>;
    if constexpr (not std::is_same_v<I, T> or not std::is_same_v<I, U>) {
      return circle<I>(*this).intersect(circle<I>(o), eps);
    } else {
      return _intersect_impl(o, epsilon<I>{eps});
    }
  }

  template <typename U>
  intersection_t<U> intersect(point<U> const& u, point<U> const& v) const {
    using I = geometry::bigger_intersection_t<T, U>;
    if constexpr (not std::is_same_v<I, T> or not std::is_same_v<I, U>) {
      return circle<I>(*this).intersect(point<I>(u), point<I>(v));
    } else {
      return _intersect_impl(u, v);
    }
  }

 private:
  intersection_t<T> _intersect_impl(circle<T> const& b) const
    requires(not std::is_integral_v<T>)  // TODO this one almost works for integral T
  {
    auto const d2 = norm(b.center - center);
    if (d2 == 0) return {radius == b.radius};
    auto const sum_r2 = (radius + b.radius) * (radius + b.radius);
    auto const dif_r2 = (radius - b.radius) * (radius - b.radius);
    if (d2 > sum_r2 or d2 < dif_r2) return {false};
    auto const d_r2 = (radius + b.radius) * (radius - b.radius);
    auto const it = ((center + b.center) + (b.center - center) * (d_r2 / d2)) / 2;
    if (d2 == sum_r2 or d2 == dif_r2) return {it};
    auto const h2d2 = radius < b.radius
                          ? d2 * radius * radius - (d2 + d_r2) * (d2 + d_r2) / 4
                          : d2 * b.radius * b.radius - (d2 - d_r2) * (d2 - d_r2) / 4;
    if (h2d2 <= 0) return {it};
    auto const shift = std::sqrt(h2d2) / d2 * perp(b.center - center);
    return {std::tuple(it - shift, it + shift)};
  }

  template <std::floating_point F = T>
    requires(std::is_same_v<F, T>)
  intersection_t<F> _intersect_impl(circle<F> const& b, epsilon<F> eps) const {
    auto const eps2 = eps * eps;
    auto const d2 = norm(b.center - center);
    if (d2 < eps2) return {std::abs(radius - b.radius) < eps};
    auto const d_r2 = (radius + b.radius) * (radius - b.radius);
    auto const it = ((center + b.center) + (b.center - center) * (d_r2 / d2)) / 2;
    auto const h2d2 = radius < b.radius
                          ? d2 * radius * radius - (d2 + d_r2) * (d2 + d_r2) / 4
                          : d2 * b.radius * b.radius - (d2 - d_r2) * (d2 - d_r2) / 4;
    if (auto const d2eps2 = d2 * eps2; h2d2 < -d2eps2) return {false};
    else if (h2d2 < d2eps2) return {it};
    auto const shift = std::sqrt(h2d2) / d2 * perp(b.center - center);
    return {std::tuple(it - shift, it + shift)};
  }

  intersection_t<T> _intersect_impl(point<T> const& u, point<T> const& v) const {
    if (std::abs(line_point_dist(u, v, center)) > radius) return {false};
    auto const mid = project(u, v, center);
    auto const h2 = radius * radius - norm(mid - center);
    if (h2 <= 0) return {mid};
    auto const shift = std::sqrt(h2 / norm(v - u)) * (v - u);
    return {std::tuple(mid - shift, mid + shift)};
  }
};

namespace geometry {
template <typename T>
  requires(point<T>::floating)
bool equal(T eps, circle<T> const& a, circle<T> const& b) {
  return equal(eps, a.center, b.center) && std::abs(a.radius - b.radius) <= eps;
}
}  // namespace geometry
