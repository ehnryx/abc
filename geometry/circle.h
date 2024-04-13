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

template <typename T>
using circle_intersection_t = circle_intersection_type<T>;

enum class tangent {
  OUTER,
  INNER,
};
}  // namespace geometry

template <typename T>
struct circle {
  using type = T;
  using product_t = point<T>::product_t;
  using intersection_t = point<T>::intersection_t;
  using circle_intersection_t = geometry::circle_intersection_t<intersection_t>;

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
  /// ie. returns [before_overlap, after_overlap] in the case of two intersections
  auto intersect(circle<T> const& b) const -> circle_intersection_t
    requires(geometry::non_floating<T>)
  {
    return circle<intersection_t>(*this).intersect(circle<intersection_t>(b));
  }
  auto intersect(circle<T> const& b) const -> circle_intersection_t
    requires(std::is_floating_point_v<T>)  // TODO this one almost works for integral T
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

  template <typename F>
  auto intersect(epsilon<F> eps, circle<T> const& b) const -> circle_intersection_t
    requires(geometry::non_floating<T>)
  {
    return circle<intersection_t>(*this).intersect(eps, circle<intersection_t>(b));
  }
  template <typename F>
  auto intersect(epsilon<F> eps, circle<T> const& b) const -> circle_intersection_t
    requires(std::is_floating_point_v<T>)
  {
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

  auto intersect(point<T> const& u, point<T> const& v) const -> circle_intersection_t
    requires(geometry::non_floating<T>)
  {
    using I = intersection_t;
    return circle<I>(*this).intersect(point<I>(u), point<I>(v));
  }
  auto intersect(point<T> const& u, point<T> const& v) const -> circle_intersection_t
    requires(std::is_floating_point_v<T>)
  {
    if (std::abs(line_point_dist(u, v, center)) > radius) return {false};
    auto const mid = project(u, v, center);
    auto const h2 = radius * radius - norm(mid - center);
    if (h2 <= 0) return {mid};
    auto const shift = std::sqrt(h2 / norm(v - u)) * (v - u);
    return {std::tuple(mid - shift, mid + shift)};
  }

  auto tangent(circle<T> const& b, geometry::tangent tangent_type) const
      -> std::pair<point<intersection_t>, point<intersection_t>>
    requires(geometry::non_floating<T>)
  {
    return circle<intersection_t>(*this).tangent(circle<intersection_t>(b), tangent_type);
  }
  auto tangent(circle<T> const& b, geometry::tangent tangent_type) const
      -> std::pair<point<T>, point<T>>
    requires(std::is_floating_point_v<T>)
  {
    auto const d = abs(center - b.center);
    auto const angle = std::asin(
        (tangent_type == geometry::tangent::OUTER ? radius - b.radius : radius + b.radius) / d);
    auto const rot = point<T>::polar(1, angle);
    auto const dir = perp(center - b.center) * rot / abs(center - b.center);
    return std::pair(
        center + dir * radius,
        b.center + (tangent_type == geometry::tangent::OUTER ? dir : -dir) * b.radius);
  }
};

namespace geometry {
template <typename T>
  requires(point<T>::floating)
bool equal(T eps, circle<T> const& a, circle<T> const& b) {
  return equal(eps, a.center, b.center) && std::abs(a.radius - b.radius) <= eps;
}
}  // namespace geometry
