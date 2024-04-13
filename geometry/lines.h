/* Lines
 * STATUS
 *  [line_inter] tested: boj/2022,6458,21048
 */
#pragma once

#include "geometry/point.h"

/// compares distance of line segment cd from line segment ab
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
    return point<I>(a) + I(dot(v - a, b - a)) / I(norm(b - a)) * point<I>(b - a);
  }
}

template <typename T>
auto line_inter(point<T> const& a, point<T> const& b, point<T> const& c, point<T> const& d) {
  using I = typename point<T>::intersection_t;
  if constexpr (std::is_same_v<T, I>) {
    return a + cross(c - a, d - c) / cross(b - a, d - c) * (b - a);
  } else {
    return point<I>(a) + I(cross(c - a, d - c)) / I(cross(b - a, d - c)) * point<I>(b - a);
  }
}

template <typename T>
auto line_point_dist(point<T> const& a, point<T> const& b, point<T> const& v) {
  using I = typename point<T>::intersection_t;
  if constexpr (std::is_same_v<T, I>) {
    return cross(b - a, v - a) / abs(b - a);
  } else {
    return I(cross(b - a, v - a)) / abs(b - a);
  }
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
