/**
 * 2d Primitives
 * TODO get error estimates on all of these
 * STATUS
 *  untested
 */
#pragma once

#include "geometry/point.h"

/// checks distance from v to line segment ab
template <std::floating_point T>
bool collinear(epsilon<T> eps, point<T> const& a, point<T> const& b, point<T> const& v) {
  return geometry::equal(eps, a, b) or std::abs(cross(b - a, v - a)) <= eps * abs(b - a);
}
template <geometry::non_floating T>
bool collinear(point<T> const& a, point<T> const& b, point<T> const& v) {
  return cross(b - a, v - a) == 0;
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
