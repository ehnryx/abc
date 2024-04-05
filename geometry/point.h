/* Point
 * USAGE
 *  point<T>(x, y);
 *  T must be integral or floating (TODO support fractions)
 *  traits (intersection_t / product_t) can be user-defined by
 *    specializing the class point_traits
 * STATUS
 *  tested
 */
#pragma once

#include "geometry/helpers.h"
#include "utility/output_tuple.h"

#include <cmath>

template <typename T>
struct point_traits : geometry::default_point_traits<T> {};

template <typename T>
struct point {
  using product_t = point_traits<T>::product_t;
  using intersection_t = point_traits<T>::intersection_t;
  T x, y;
  constexpr point() : x(0), y(0) {}
  constexpr point(T const& _x, T const& _y) : x(_x), y(_y) {}
  template <typename U>
    requires(geometry::implicit_conversion_v<T, U>)
  constexpr point(point<U> const& v) : x(v.x), y(v.y) {}
  template <typename U>
    requires(not geometry::implicit_conversion_v<T, U>)
  constexpr explicit point(point<U> const& v) : x(v.x), y(v.y) {}
  template <typename U, typename V>
  static auto polar(U const& radius, V const& angle) -> point
    requires(std::is_floating_point_v<T>)
  {
    return point(radius * std::cos(angle), radius * std::sin(angle));
  }

  friend std::ostream& operator<<(std::ostream& os, point const& v) {
    return os << "(" << v.x << "," << v.y << ")";
  }
  friend std::istream& operator>>(std::istream& is, point& v) { return is >> v.x >> v.y; }
  auto operator*() const { return output_tuple(x, y); }

  bool operator==(point const& v) const { return x == v.x and y == v.y; }
  bool operator<(point const& v) const { return std::tie(x, y) < std::tie(v.x, v.y); }
  bool operator>(point const& v) const { return std::tie(x, y) > std::tie(v.x, v.y); }
  point operator-() const { return point(-x, -y); }
  point operator+(point const& v) const { return point(x + v.x, y + v.y); }
  point operator-(point const& v) const { return point(x - v.x, y - v.y); }
  point operator*(point const& v) const { return point(x * v.x - y * v.y, x * v.y + y * v.x); }
  point operator/(point const& v) const { return (*this) * v.inverse(); }
  point operator*(T const& c) const { return point(x * c, y * c); }
  point operator/(T const& c) const { return point(x / c, y / c); }
  // clang-format off
  point& operator+=(point const& v) { x += v.x; y += v.y; return *this; }
  point& operator-=(point const& v) { x -= v.x; y -= v.y; return *this; }
  point& operator*=(point const& v) { return *this = point(x * v.x - y * v.y, x * v.y + y * v.x); }
  point& operator/=(point const& v) { return operator*=(v.inverse()); }
  point& operator*=(T const& c) { x *= c; y *= c; return *this; }
  point& operator/=(T const& c) { x /= c; y /= c; return *this; }
  // clang-format on
  friend point operator*(T const& c, point const& v) { return v * c; }
  friend point operator/(T const& c, point const& v) { return v.inverse() * c; }

  point transpose() const { return point(y, x); }
  point inverse() const { return conj() / norm(); }
  point conj() const { return point(x, -y); }
  point perp() const { return point(-y, x); }
  product_t norm() const { return product_t(x) * x + product_t(y) * y; }
  product_t dot(point const& v) const { return product_t(x) * v.x + product_t(y) * v.y; }
  product_t cross(point const& v) const { return product_t(x) * v.y - product_t(y) * v.x; }
  intersection_t abs() const { return std::sqrt(norm()); }
  intersection_t absl() const { return sqrtl(norm()); }
  intersection_t arg() const { return std::atan2(y, x); }
  intersection_t argl() const { return atan2l(y, x); }
  T manhattan() const { return std::abs(x) + std::abs(y); }

  // similar interface to std::complex
  T real() const { return x; }
  T imag() const { return y; }
  void real(T const& v) { x = v; }
  void imag(T const& v) { y = v; }

  // sort helpers
  static auto by_angle(point const& a, point const& b) -> bool { return a.arg() < b.arg(); }
  static auto by_x(point const& a, point const& b) -> bool {
    return a.x < b.x or (a.x == b.x && a.y < b.y);
  }
  static auto by_y(point const& a, point const& b) -> bool {
    return a.y < b.y or (a.y == b.y && a.x < b.x);
  }
  static auto tiebreak_by_norm(const point& u, const point& v) -> bool {
    return u.norm() < v.norm();
  }
  /// neither u nor v should be (0, 0) in the comparisons
  /// default: break ties by distance from origin
  template <typename Function = decltype(tiebreak_by_norm)>
  static auto ccw_from_ref(point const& ref, Function const& tiebreak = tiebreak_by_norm) {
    // neither u nor v should be ref
    return [ref, tiebreak](point const& u, point const& v) {
      auto const cu = ref.cross(u);
      auto const cv = ref.cross(v);
      bool const u_pos = cu > 0 or (cu == 0 and ref.dot(u) > 0);
      bool const v_pos = cv > 0 or (cv == 0 and ref.dot(v) > 0);
      if (u_pos != v_pos) return u_pos;
      auto const turn = u.cross(v);
      return turn > 0 or (turn == 0 and tiebreak(u, v));
    };
  }
};

// clang-format off
template <typename T> auto real(point<T> const& v) { return v.real(); }
template <typename T> auto imag(point<T> const& v) { return v.imag(); }
template <typename T> auto conj(point<T> const& v) { return v.conj(); }
template <typename T> auto perp(point<T> const& v) { return v.perp(); }
template <typename T> auto norm(point<T> const& v) { return v.norm(); }
template <typename T> auto arg(point<T> const& v) { return v.arg(); }
template <typename T> auto abs(point<T> const& v) { return v.abs(); }
template <typename T> auto argl(point<T> const& v) { return v.argl(); }
template <typename T> auto absl(point<T> const& v) { return v.absl(); }
template <typename T> auto dot(point<T> const& a, point<T> const& b) { return a.dot(b); }
template <typename T> auto cross(point<T> const& a, point<T> const& b) { return a.cross(b); }
template <typename T> auto manhattan(point<T> const& v) { return v.manhattan(); }
// clang-format on

namespace geometry {
template <std::floating_point T>
bool equal(epsilon<T> eps, point<T> const& a, point<T> const& b) {
  return abs(a - b) <= eps;
}
template <std::floating_point T>
bool less_than(epsilon<T> eps, point<T> const& a, point<T> const& b) {
  return a.x + eps < b.x or (a.x <= b.x + eps && a.y + eps < b.y);
}
}  // namespace geometry

#include "utility/fast_input_read.h"

template <typename T>
struct fast_input_read<point<T>> {
  template <typename input_t>
  static void get(input_t& in, point<T>& v) {
    in >> v.x >> v.y;
  }
};
