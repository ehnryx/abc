/* Point
 * USAGE
 *  point<T, D>(...);
 *  T must be integral or floating
 * STATUS
 *  untested
 */
#pragma once

#include "utility/output_tuple.h"

#include <array>
#include <iostream>

template <typename T, size_t Dimension>
  requires(std::is_floating_point_v<T> or std::is_integral_v<T>)
struct point_nd {
  static constexpr auto dimension_indices = std::make_index_sequence<Dimension>();
  std::array<T, Dimension> xs = {};
  point_nd() = default;
  template <typename... Args>
  point_nd(Args... coords) : xs{coords...} {}
  template <typename U>
  explicit point_nd(point_nd<U, Dimension> const& other) : point_nd(other, dimension_indices) {}
  template <typename U, size_t... Is>
  explicit point_nd(point_nd<U, Dimension> const& other, std::index_sequence<Is...>)
      : xs{T(other[Is])...} {}
  auto operator[](size_t i) -> T& { return xs[i]; }
  auto operator[](size_t i) const -> T { return xs[i]; }
  auto operator+(point_nd const& o) const -> point_nd { return point_nd(*this) += o; }
  auto operator-(point_nd const& o) const -> point_nd { return point_nd(*this) -= o; }
  auto operator*(T c) const -> point_nd { return point_nd(*this) *= c; }
  auto operator/(T c) const -> point_nd { return point_nd(*this) /= c; }
  auto operator-() const -> point_nd { return point_nd(*this).negate(); }
  auto operator+=(point_nd const& o) -> point_nd& {
    for (size_t i = 0; i < xs.size(); i++) xs[i] += o.xs[i];
    return *this;
  }
  auto operator-=(point_nd const& o) -> point_nd& {
    for (size_t i = 0; i < xs.size(); i++) xs[i] -= o.xs[i];
    return *this;
  }
  auto operator*=(T c) -> point_nd& {
    for (size_t i = 0; i < xs.size(); i++) xs[i] *= c;
    return *this;
  }
  auto operator/=(T c) -> point_nd& {
    for (size_t i = 0; i < xs.size(); i++) xs[i] /= c;
    return *this;
  }
  auto negate() -> point_nd& {
    for (size_t i = 0; i < xs.size(); i++) xs[i] = -xs[i];
    return *this;
  }
  auto dot(point_nd const& other) const -> T
    requires(std::is_floating_point_v<T>)  // i don't want to think about overflow
  {
    T res = 0;
    for (size_t i = 0; i < xs.size(); i++) {
      res += xs[i] * other.xs[i];
    }
    return res;
  }
  auto norm() const -> T { return dot(*this); }
  friend auto operator<<(std::ostream& os, point_nd const& v) -> std::ostream& {
    os << "(" << v[0];
    for (size_t i = 1; i < Dimension; i++) {
      os << "," << v[i];
    }
    return os << ")";
  }
  friend auto operator>>(std::istream& is, point_nd& v) -> std::istream& {
    for (size_t i = 0; i < Dimension; i++) {
      is >> v[i];
    }
    return is;
  }
  auto operator*() const { return make_output_tuple(xs); }
};

template <typename T, size_t D>
auto norm(point_nd<T, D> const& v) -> T {
  return v.norm();
}
