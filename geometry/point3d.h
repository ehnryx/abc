/* Point 3d
 * USAGE
 *  point3d<T>(x, y, z);
 *  T must be floating point
 * STATUS
 *  tested
 */
#pragma once

#include "geometry/helpers.h"
#include "utility/output_tuple.h"

#include <cmath>

template <typename T>
struct point3d_traits : geometry::default_point_traits<T> {};

template <typename T>
struct point3d {
  using product_t = point3d_traits<T>::product_t;
  using intersection_t = point3d_traits<T>::intersection_t;
  T x, y, z;
  constexpr point3d() = default;
  constexpr point3d(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
  template <typename U>
    requires(geometry::implicit_conversion_v<T, U>)
  constexpr point3d(point3d<U> const& o) : x(o.x), y(o.y), z(o.z) {}
  template <typename U>
    requires(not geometry::implicit_conversion_v<T, U>)
  constexpr explicit point3d(point3d<U> const& o) : x(o.x), y(o.y), z(o.z) {}

  friend auto operator<<(std::ostream& os, point3d const& v) -> std::ostream& {
    return os << "(" << v.x << "," << v.y << "," << v.z << ")";
  }
  friend auto operator>>(std::istream& is, point3d& v) -> std::istream& {
    return is >> v.x >> v.y >> v.z;
  }
  auto operator*() const { return output_tuple(x, y, z); }

  // TODO broken?
  template <typename U, typename V, typename W>
  static auto polar(U const& radius, V const& polar_angle, W const& azimuthal_angle) -> point3d
    requires(std::is_floating_point_v<T>)
  {
    return point3d{
        radius * std::sin(polar_angle) * std::cos(azimuthal_angle),
        radius * std::sin(polar_angle) * std::sin(azimuthal_angle),
        radius * std::cos(polar_angle),
    };
  }
  auto polar_angle() const -> intersection_t {
    return std::atan2(std::sqrt(intersection_t(product_t(x) * x + product_t(y) * y)), z);
  }
  auto azimuthal_angle() const -> intersection_t { return std::atan2(y, x); }

  // clang-format off
  auto operator==(point3d const& o)  const -> bool { return x == o.x && y == o.y && z == o.z; }
  auto operator<(point3d const& o) const -> bool { return std::tie(x, y, z) < std::tie(o.x, o.y, o.z); }
  auto operator>(point3d const& o) const -> bool { return std::tie(x, y, z) > std::tie(o.x, o.y, o.z); }
  // clang-format on

  auto is_zero() const -> bool { return x == 0 and y == 0 and z == 0; }
  auto operator+(point3d const& o) const -> point3d { return point3d(*this) += o; }
  auto operator-(point3d const& o) const -> point3d { return point3d(*this) -= o; }
  auto operator*(T c) const -> point3d { return point3d(*this) *= c; }
  auto operator/(T c) const -> point3d { return point3d(*this) /= c; }
  auto operator-() const -> point3d { return point3d(*this).negate(); }
  friend auto operator*(T c, point3d const& v) -> point3d { return v * c; }
  // clang-format off
  auto operator+=(point3d const& o) -> point3d& { x += o.x; y += o.y; z += o.z; return *this; }
  auto operator-=(point3d const& o) -> point3d& { x -= o.x; y -= o.y; z -= o.z; return *this; }
  auto operator*=(T c) -> point3d& { x *= c; y *= c; z *= c; return *this; }
  auto operator/=(T c) -> point3d& { x /= c; y /= c; z /= c; return *this; }
  auto negate() -> point3d& { x = -x; y = -y; z = -z; return *this; }
  // clang-format on
  auto dot(point3d const& o) const -> product_t {
    return product_t(x) * o.x + product_t(y) * o.y + product_t(z) * o.z;
  }
  auto norm() const -> product_t { return dot(*this); }
  auto abs() const -> intersection_t { return std::sqrt(intersection_t(norm())); }
  auto cross(point3d const& o) const -> point3d<product_t> {
    return point3d<product_t>{
        product_t(y) * o.z - product_t(z) * o.y,
        product_t(z) * o.x - product_t(x) * o.z,
        product_t(x) * o.y - product_t(y) * o.x,
    };
  }

  /// points u and v lie on the axis of rotation. angle respects right-hand-rule
  auto rotate(point3d const& u, point3d const& v, T angle) const -> point3d
    requires(std::is_floating_point_v<T>)
  {
    return (*this - u).rotate(v - u, angle) + u;
  }
  /// axis does not need to be unit, but must be nonzero
  auto rotate(point3d const& axis, T angle) const -> point3d
    requires(std::is_floating_point_v<T>)
  {
    return rotate_unit(axis / axis.abs(), angle);
  }
  /// requires that axis is unit. angle respects right-hand-rule
  auto rotate_unit(point3d const& unit_axis, T angle) const -> point3d
    requires(std::is_floating_point_v<T>)
  {
    auto const c = std::cos(angle);
    auto const s = std::sin(angle);
    auto res = unit_axis * ((1 - c) * dot(unit_axis)) + *this * c - cross(unit_axis) * s;
    auto other = point3d{
        x * ((1 - c) * unit_axis.x * unit_axis.x + c) +
            y * ((1 - c) * unit_axis.x * unit_axis.y - s * unit_axis.z) +
            z * ((1 - c) * unit_axis.x * unit_axis.z + s * unit_axis.y),
        x * ((1 - c) * unit_axis.x * unit_axis.y + s * unit_axis.z) +
            y * ((1 - c) * unit_axis.y * unit_axis.y + c) +
            z * ((1 - c) * unit_axis.y * unit_axis.z - s * unit_axis.x),
        x * ((1 - c) * unit_axis.x * unit_axis.z - s * unit_axis.y) +
            y * ((1 - c) * unit_axis.y * unit_axis.z + s * unit_axis.x) +
            z * ((1 - c) * unit_axis.z * unit_axis.z + c),
    };
    assert(abs(res - other) < 1e-9);
    return res;
  }
};

// clang-format off
template <typename T> auto abs(point3d<T> const& v) { return v.abs(); }
template <typename T> auto norm(point3d<T> const& v) { return v.norm(); }
template <typename T> auto dot(point3d<T> const& a, point3d<T> const& b) { return a.dot(b); }
template <typename T> auto cross(point3d<T> const& a, point3d<T> const& b) { return a.cross(b); }
// clang-format on

#include "utility/named_types.h"

namespace geometry {
template <std::floating_point T>
bool equal(epsilon<T> eps, point3d<T> const& a, point3d<T> const& b) {
  return abs(a - b) <= eps;
}
}  // namespace geometry
