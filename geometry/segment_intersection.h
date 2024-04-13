/** Segment Intersection
 * STATUS
 *  tested: boj/15010,17386
 */
#pragma once

#include "geometry/point.h"
#include "geometry/primitives.h"

template <std::floating_point T>
bool seg_x_seg(
    epsilon<T> eps, point<T> const& a, point<T> const& b, point<T> const& c, point<T> const& d,
    strict strict = {false}) {
  auto const ab = abs(a - b);
  if (ab <= eps) return not strict && on_segment(eps, c, d, a);
  auto const cd = abs(c - d);
  if (cd <= eps) return not strict && on_segment(eps, a, b, d);
  int const r1 = geometry::sign(epsilon{eps * ab}, cross(b - a, c - a));
  int const r2 = geometry::sign(epsilon{eps * ab}, cross(b - a, d - a));
  if (r1 == 0 && r2 == 0) {
    return strict ? geometry::less_than(eps, std::min(a, b), std::max(c, d)) &&
                        geometry::less_than(eps, std::min(c, d), std::max(a, b))
                  : not(geometry::less_than(eps, std::max(a, b), std::min(c, d)) ||
                        geometry::less_than(eps, std::max(c, d), std::min(a, b)));
  }
  int const r3 = geometry::sign(epsilon{eps * cd}, cross(d - c, a - c));
  int const r4 = geometry::sign(epsilon{eps * cd}, cross(d - c, b - c));
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
