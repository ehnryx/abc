/* Segment
 * USAGE
 *  segment<T>(u, v);
 *  T must be integral or floating (TODO support fractions)
 * STATUS
 *  tested: boj/3108,11662
 */
#pragma once

#include "geometry/lines.h"
#include "geometry/point.h"

template <typename T>
struct segment {
  point<T> start, end;
  segment() = default;
  segment(point<T> const& s, point<T> const& e) : start(s), end(e) {}
  template <typename U>
  explicit segment(segment<U> const& o) : start(o.start), end(o.end) {}
  auto direction() const -> point<T> { return end - start; }
  template <typename F>
  auto get(F t) const -> point<F> {
    return point<F>(start.x * (1 - t) + end.x * t, start.y * (1 - t) + end.y * t);
  }
  template <typename F>
  auto get(point<F> const& v) const -> point<T>::intersection_t {
    return dot(v - point<F>(start), point<F>(direction())) /
           typename point<T>::intersection_t(direction().norm());
  }
  auto operator<=>(segment const& o) const -> bool {
    return std::tie(start, end) <=> std::tie(o.start, o.end);
  }
  friend std::ostream& operator<<(std::ostream& os, segment const& s) {
    return os << s.start << " -- " << s.end;
  }
  friend std::istream& operator>>(std::istream& is, segment& s) {
    return is >> s.start >> s.end;
  }

  // clang-format off
  auto operator+=(point<T> const& v) -> segment& { start += v; end += v; return *this; }
  auto operator-=(point<T> const& v) -> segment& { start -= v; end -= v; return *this; }
  auto operator+(point<T> const& v) const -> segment { return segment(*this) += v; }
  auto operator-(point<T> const& v) const -> segment { return segment(*this) -= v; }
  // clang-format on
};

template <typename T>
auto line_inter(segment<T> const& ab, segment<T> const& cd) {
  return line_inter(ab.start, ab.end, cd.start, cd.end);
}

template <std::floating_point T>
bool seg_x_seg(
    epsilon<T> eps, segment<T> const& ab, segment<T> const& cd, strict strict = {false}) {
  return seg_x_seg(eps, ab.start, ab.end, cd.start, cd.end, strict);
}
template <geometry::non_floating T>
bool seg_x_seg(segment<T> const& ab, segment<T> const& cd, strict strict = {false}) {
  return seg_x_seg(ab.start, ab.end, cd.start, cd.end, strict);
}
