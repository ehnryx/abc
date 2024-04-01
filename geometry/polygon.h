/* Polygon
 * STATUS
 *  [signed_area] tested: boj/2166,18298
 */
#pragma once

#include "geometry/lines.h"
#include "geometry/point.h"

#include <vector>

template <typename T>
struct polygon {
  std::vector<point<T>> p;
  polygon(std::initializer_list<point<T>>&& _p) : p(std::move(_p)) {}
  polygon(std::vector<point<T>>&& _p) : p(std::move(_p)) {}
  polygon(std::vector<point<T>> const& _p) : p(_p) {}
  auto operator[](int i) -> T& { return p[i]; }
  auto operator[](int i) const -> T const& { return p[i]; }
  auto size() const -> int { return static_cast<int>(p.size()); }

  auto signed_area() const { return doubled_area() / 2.0; }
  auto doubled_area() const {
    auto area = typename point<T>::product_t{0};
    for (int i = size() - 1, j = 0; j < size(); i = j++) {
      area += cross(p[i], p[j]);
    }
    return area;
  }

  template <std::floating_point F = T>
  bool on_boundary(epsilon<F> eps, point<T> const& c) const {
    bool ok = false;
    for (int i = size() - 1, j = 0; not ok && j < size(); i = j++) {
      ok |= on_segment(eps, p[i], p[j], c);
    }
    return ok;
  }
  bool on_boundary(const point<T>& c) const
    requires(not std::is_floating_point_v<T>)
  {
    bool ok = false;
    for (int i = size() - 1, j = 0; !ok && j < size(); i = j++) {
      ok |= on_segment(p[i], p[j], c);
    }
    return ok;
  }

  template <std::floating_point F = T>
  bool contains(epsilon<F> eps, const point<T>& c, strict strict = {false}) const {
    if (on_boundary(eps, c)) return not strict;
    double sum = 0;
    for (int i = size() - 1, j = 0; j < size(); i = j++) {
      sum += atan2(cross(p[i] - c, p[j] - c), dot(p[i] - c, p[j] - c));
    }
    return std::abs(sum) > 1;
  }
  bool contains(const point<T>& c, strict strict = {false}) const
    requires(not std::is_floating_point_v<T>)
  {
    if (on_boundary(c)) return not strict;
    double sum = 0;
    for (int i = size() - 1, j = 0; j < size(); i = j++) {
      sum += atan2(cross(p[i] - c, p[j] - c), dot(p[i] - c, p[j] - c));
    }
    return std::abs(sum) > 1;
  }

  friend std::ostream& operator<<(std::ostream& os, polygon const& gon) {
    os << "[ ";
    for (size_t i = 0; i < gon.size(); i++) {
      os << gon[i] << ", ";
    }
    return os << "]";
  }
};
