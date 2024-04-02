/* Polygon
 * STATUS
 *  [signed_area] tested: boj/2166,18298
 */
#pragma once

#include "geometry/lines.h"
#include "geometry/point.h"
#include "geometry/segment.h"

#include <algorithm>
#include <vector>

template <typename T>
struct polygon {
  std::vector<point<T>> p;
  polygon(int n = 0) : p(n) {}
  polygon(std::initializer_list<point<T>>&& _p) : p(std::move(_p)) {}
  polygon(std::vector<point<T>>&& _p) : p(std::move(_p)) {}
  polygon(std::vector<point<T>> const& _p) : p(_p) {}
  template <typename F>
  explicit polygon(polygon<F> const& other) : p(other.p.begin(), other.p.end()) {}
  auto operator[](int i) -> point<T>& { return p[i]; }
  auto operator[](int i) const -> point<T> const& { return p[i]; }
  auto size() const -> int { return static_cast<int>(p.size()); }
  auto empty() const -> bool { return p.empty(); }
  auto reverse() -> void { std::reverse(p.begin(), p.end()); }

  auto edges() -> std::vector<segment<T>> {
    std::vector<segment<T>> res;
    for (int prev_i = size() - 1, i = 0; i < size(); prev_i = i++) {
      res.emplace_back(p[prev_i], p[i]);
    }
    return res;
  }

  auto signed_area() const {
    if constexpr (std::is_integral_v<T>) return double(doubled_area()) / 2.0;
    else return doubled_area() / 2.0;
  }
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
  friend std::istream& operator>>(std::istream& is, polygon& gon) {
    if (gon.empty()) {
      int n;
      is >> n;
      gon.p.resize(n);
    }
    for (int i = 0; i < gon.size(); i++) {
      is >> gon.p[i];
    }
    return is;
  }
};
