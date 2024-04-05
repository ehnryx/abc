/** Minimum Enclosing Ball
 *  source: Fast and Robust Smallest Enclosing Balls (Bernd Gaertner)
 *  https://people.inf.ethz.ch/gaertner/subdir/texts/own_work/esa99_final.pdf
 *  https://people.inf.ethz.ch/gaertner/subdir/software/miniball.html
 * USAGE
 *  auto mb = miniball<type, dimension>(iterable of point_nd<type, dimension>);
 * MEMBERS
 *  center()
 *  radius()
 *  squared_radius()
 * STATUS
 *  tested: boj/9158,11930,14507,21182
 */
#pragma once

#include "geometry/point_nd.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <list>

template <std::floating_point T, unsigned Dimension>
  requires(Dimension > 0)
struct miniball {
  using point_t = point_nd<T, Dimension>;

  std::array<point_t, Dimension + 1> c = {};  // centers
  std::array<T, Dimension + 1> r2 = {T(-1)};  // squared radii

  std::array<point_t, Dimension + 1> v;                   // helper
  std::array<T, Dimension + 1> z;                         // helper
  std::array<T, Dimension + 1> f;                         // helper
  std::array<std::array<T, Dimension>, Dimension + 1> A;  // helper

  std::list<unsigned> L;   // list of support points
  unsigned n_forced = 0;   // number of forced points
  unsigned n_support = 0;  // number of support points
  unsigned last_i = 0;     // current center and radius

  std::vector<point_t> const ps;  // all points

  auto center() const -> point_t { return c[last_i]; }
  auto radius() const -> T { return std::sqrt(r2[last_i]); }
  auto squared_radius() const -> T { return r2[last_i]; }

  template <std::input_iterator InputIt>
  miniball(InputIt first, InputIt last) : ps(first, last) {
    build();
  }
  template <typename Container>
    requires(requires(Container const& cont) {
      std::begin(cont);
      std::end(cont);
    })
  miniball(Container const& cont) : miniball(std::begin(cont), std::end(cont)) {}

  auto build() -> void {
    auto support_end = L.begin();  // L .. support_end is at most (Dimension + 1)
    unsigned stale = 0;
    do {
      auto const old_r2 = squared_radius();
      auto max_e = T(0);
      unsigned pivot = 0;
      for (unsigned k = 0; k < ps.size(); k++) {
        auto const e = norm(ps[k] - center()) - old_r2;
        if (e > max_e) {
          max_e = e;
          pivot = k;
        }
      }

      if (old_r2 < 0 || max_e > 0) {
        if (std::find(L.begin(), support_end, pivot) == support_end) {
          if (push(pivot)) {
            support_end = move_to_front_mb(support_end);
            pop();
            L.push_front(pivot);
            if (std::distance(L.begin(), support_end) == Dimension + 2) support_end--;
          }
        }
      }
      if (squared_radius() > old_r2) stale = 0;
      else stale++;
    } while (stale < 2);
    // TODO is this stale counter necessary or can I just check latest_r2 > old_r2?
  }

  auto move_to_front_mb(std::list<unsigned>::iterator n) -> std::list<unsigned>::iterator {
    auto support_end = L.begin();
    if (n_forced == Dimension + 1) return support_end;
    for (auto i = L.begin(); i != n;) {
      auto const j = i++;
      if (norm(ps[*j] - center()) - squared_radius() > 0) {
        if (push(*j)) {
          support_end = move_to_front_mb(j);
          pop();
          if (support_end == j) support_end++;
          L.splice(L.begin(), L, j);
        }
      }
    }
    return support_end;
  }

  auto pop() -> void { --n_forced; }
  auto push(unsigned it) -> bool {
    static constexpr auto eps =
        std::numeric_limits<T>::epsilon() * std::numeric_limits<T>::epsilon();
    if (n_forced == 0) {
      c[0] = ps[it];
      r2[0] = 0;
    } else {
      v[n_forced] = ps[it] - c[0];
      for (unsigned i = 1; i < n_forced; i++) {
        A[n_forced][i] = v[n_forced].dot(v[i]) * 2 / z[i];
      }
      for (unsigned i = 1; i < n_forced; i++) {
        v[n_forced] -= v[i] * A[n_forced][i];
      }
      z[n_forced] = norm(v[n_forced]) * 2;
      if (z[n_forced] < eps * squared_radius()) {
        return false;
      }
      auto const e = norm(ps[it] - c[n_forced - 1]) - r2[n_forced - 1];
      f[n_forced] = e / z[n_forced];
      c[n_forced] = c[n_forced - 1] + v[n_forced] * f[n_forced];
      r2[n_forced] = r2[n_forced - 1] + e * f[n_forced] / 2;
    }
    last_i = n_forced;
    n_support = ++n_forced;
    return true;
  }
};
