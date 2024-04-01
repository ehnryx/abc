/* Lines
 * STATUS
 *  [seg_x_seg] tested: boj/15010,17386
 *  [line_inter] tested: boj/2022,6458
 */
#pragma once

#include "geometry/point.h"

template <std::floating_point T>
bool collinear(epsilon<T> eps, point<T> const& a, point<T> const& b, point<T> const& v) {
  return geometry::equal(eps, a, v) or std::abs(cross(a - v, b - v)) <= eps * abs(a - v);
}
template <geometry::non_floating T>
bool collinear(point<T> const& a, point<T> const& b, point<T> const& v) {
  return cross(a - v, b - v) == 0;
}

template <std::floating_point T>
bool parallel(
    epsilon<T> eps, point<T> const& a, point<T> const& b, point<T> const& c,
    point<T> const& d) {
  return geometry::equal(eps, a, b) || std::abs(cross(a - b, c - d)) <= eps * abs(a - b);
}
template <geometry::non_floating T>
bool parallel(point<T> const& a, point<T> const& b, point<T> const& c, point<T> const& d) {
  return cross(a - b, c - d) == 0;
}

template <typename T>
auto project(point<T> const& a, point<T> const& b, point<T> const& v) {
  using I = typename point<T>::intersection_t;
  if constexpr (std::is_same_v<T, I>) {
    return a + dot(v - a, b - a) / norm(b - a) * (b - a);
  } else {
    return point<I>(a) + point<I>(b - a) * dot(v - a, b - a) / norm(b - a);
  }
}

template <typename T>
auto line_inter(point<T> const& a, point<T> const& b, point<T> const& c, point<T> const& d) {
  using I = typename point<T>::intersection_t;
  if constexpr (std::is_same_v<T, I>) {
    return a + cross(c - a, d - c) / cross(b - a, d - c) * (b - a);
  } else {
    return point<I>(a) + point<I>(b - a) * cross(c - a, d - c) / cross(b - a, d - c);
  }
}

template <typename T>
auto line_point_dist(point<T> const& a, point<T> const& b, point<T> const& v) {
  return cross(b - a, v - a) / abs(b - a);
}

template <typename T>
auto segment_point_dist(point<T> const& a, point<T> const& b, point<T> const& v) {
  if (dot(b - a, v - a) > 0 && dot(a - b, v - b) > 0) {
    return std::abs(line_point_dist(a, b, v));
  } else {
    return std::min(abs(a - v), abs(b - v));
  }
}

template <typename T>
auto segment_closest(point<T> const& a, point<T> const& b, point<T> const& v) {
  if (dot(b - a, v - a) > 0 && dot(a - b, v - b) > 0) {
    return line_inter(a, b, v, v + perp(a - b));
  } else {
    return point<typename point<T>::intersection_t>(norm(a - v) < norm(b - v) ? a : b);
  }
}

template <std::floating_point T>
bool on_segment(
    epsilon<T> eps, point<T> const& a, point<T> const& b, point<T> const& v,
    strict strict = {false}) {
  if (geometry::equal(eps, a, v) || geometry::equal(eps, b, v)) return not strict;
  return collinear(eps, a, b, v) && dot(b - a, v - a) > 0 && dot(a - b, v - b) > 0;
}
template <geometry::non_floating T>
bool on_segment(
    point<T> const& a, point<T> const& b, point<T> const& v, strict strict = {false}) {
  if (a == v || b == v) return not strict;
  return collinear(a, b, v) && dot(b - a, v - a) > 0 && dot(a - b, v - b) > 0;
}

template <std::floating_point T>
bool seg_x_seg(
    epsilon<T> eps, point<T> const& a, point<T> const& b, point<T> const& c, point<T> const& d,
    strict strict = {false}) {
  auto const ab = abs(a - b);
  if (ab <= eps) return not strict && on_segment(eps, c, d, a);
  auto const cd = abs(c - d);
  if (cd <= eps) return not strict && on_segment(eps, a, b, d);
  int const r1 = geometry::sign(cross(b - a, c - a), epsilon{eps * ab});
  int const r2 = geometry::sign(cross(b - a, d - a), epsilon{eps * ab});
  if (r1 == 0 && r2 == 0) {
    return strict ? geometry::less_than(eps, std::min(a, b), std::max(c, d)) &&
                        geometry::less_than(eps, std::min(c, d), std::max(a, b))
                  : not(geometry::less_than(eps, std::max(a, b), std::min(c, d)) ||
                        geometry::less_than(eps, std::max(c, d), std::min(a, b)));
  }
  int const r3 = geometry::sign(cross(d - c, a - c), epsilon{eps * cd});
  int const r4 = geometry::sign(cross(d - c, b - c), epsilon{eps * cd});
  return strict ? r1 * r2 < 0 && r3 * r4 < 0 : r1 * r2 <= 0 && r3 * r4 <= 0;
}
template <geometry::non_floating T>
bool seg_x_seg(
    point<T> const& a, point<T> const& b, point<T> const& c, point<T> const& d,
    strict strict = {false}) {
  if (a == b) return not strict && on_segment(c, d, a, strict);
  if (c == d) return not strict && on_segment(a, b, d, strict);
  int const r1 = geometry::sign(cross(b - a, c - a));
  int const r2 = geometry::sign(cross(b - a, d - a));
  if (r1 == 0 && r2 == 0) {
    return strict ? std::min(a, b) < std::max(c, d) && std::min(c, d) < std::max(a, b)
                  : not(std::max(a, b) < std::min(c, d) || std::max(c, d) < std::min(a, b));
  }
  int const r3 = geometry::sign(cross(d - c, a - c));
  int const r4 = geometry::sign(cross(d - c, b - c));
  return strict ? r1 * r2 < 0 && r3 * r4 < 0 : r1 * r2 <= 0 && r3 * r4 <= 0;
}
