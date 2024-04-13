/* 3D Convex Hull (randomized Quickhull or incremental)
 * USAGE
 *  convex_hull<T>(begin(points), end(points));
 * INPUT
 *  points: iterable of point3d<T>
 * OUTPUT
 *  faces should be oriented outwards (right-hand-rule)
 *  faces should not have interior points
 * STATUS
 *  tested: boj/4795,5401,7684,14507
 */
#pragma once

#include "geometry/convex_hull.h"
#include "geometry/primitives_3d.h"
#include "utility/named_types.h"
#include "utility/random.h"

#include <algorithm>
#include <vector>

namespace convex_hull_details {
struct edge {
  int const to;   // destination of half-edge
  int face = -1;  // incident face index
};
struct face {
  int ei, ej, ek;  // incident edge indices
  int removed = false;
};
template <typename T>
struct point {
  point3d<T> v;  // actual point
  int index;     // original index of this point
  auto operator->() const -> point3d<T> const* { return &v; }
  auto operator*() const -> point3d<T> const& { return v; }
  auto operator<(point const& o) const -> bool { return v < o.v; }
  auto operator==(point const& o) const -> bool { return v == o.v; }
};

template <typename T>
struct epsilon_base {
  auto check_equal(auto const& a, auto const& b) { return a == b; };
  auto check_collinear(auto const&... args) { return collinear(args...); }
  auto check_orientation(auto const&... args) { return plane_point_orientation(args...); }
  auto do_convex_hull(auto const&... args) { return convex_hull(args...); }
  auto get_2d_points(auto const& normal, auto const& ps_3d) -> std::vector<::point<T>> {
    std::vector<::point<T>> p2d;
    p2d.reserve(ps_3d.size());
    if (auto dx = normal.x < 0 ? -normal.x : normal.x, dy = normal.y < 0 ? -normal.y : normal.y,
        dz = normal.z < 0 ? -normal.z : normal.z;
        dz >= dx and dz >= dy) {
      for (auto const& v : ps_3d) {
        p2d.emplace_back(v->x, v->y);
      }
    } else if (dy >= dx) {
      for (auto const& v : ps_3d) {
        p2d.emplace_back(v->x, v->z);
      }
    } else {
      for (auto const& v : ps_3d) {
        p2d.emplace_back(v->y, v->z);
      }
    }
    std::sort(p2d.begin(), p2d.end());
    return p2d;
  }
};
template <std::floating_point T>
struct epsilon_base<T> {
  epsilon<T> const eps;
  auto check_equal(auto const&... args) { return geometry::equal(eps, args...); };
  auto check_collinear(auto const&... args) { return collinear(eps, args...); }
  auto check_orientation(auto const&... args) { return plane_point_orientation(eps, args...); }
  auto do_convex_hull(auto const&... args) { return convex_hull(eps, args...); }
  auto get_2d_points(auto const& normal, auto const& ps_3d) -> std::vector<::point<T>> {
    auto rot = rotation_matrix(normal, point3d<T>(0, 0, 1));
    std::vector<::point<T>> p2d;
    p2d.reserve(ps_3d.size());
    for (auto const& v : ps_3d) {
      auto rv = rot(*v);
      p2d.emplace_back(rv.x, rv.y);
    }
    std::sort(p2d.begin(), p2d.end());
    return p2d;
  }
};
}  // namespace convex_hull_details

template <typename derived_t, typename T>
struct convex_hull_3d_base : convex_hull_details::epsilon_base<T> {
  std::vector<convex_hull_details::point<T>> ps;
  std::vector<convex_hull_details::edge> edges;
  std::vector<convex_hull_details::face> faces;
  int dimension = 0;

  template <std::floating_point F = T, std::input_iterator InputIt>
  convex_hull_3d_base(epsilon<F> e, InputIt first, InputIt last)
      : convex_hull_details::epsilon_base<T>(e), ps(first, last) {
    _build();
  }
  template <std::floating_point F = T, typename Container>
    requires(requires(Container const& cont) {
      std::begin(cont);
      std::end(cont);
    })
  convex_hull_3d_base(epsilon<F> e, Container const& cont)
      : convex_hull_3d_base(e, std::begin(cont), std::end(cont)) {}

  template <std::input_iterator InputIt>
  convex_hull_3d_base(InputIt first, InputIt last) : ps(first, last) {
    _build();
  }
  template <typename Container>
    requires(requires(Container const& cont) {
      std::begin(cont);
      std::end(cont);
    })
  convex_hull_3d_base(Container const& cont)
      : convex_hull_3d_base(std::begin(cont), std::end(cont)) {}

  auto is_delaunay(convex_hull_details::face const& f) const -> bool {
    auto const& A = *ps[edges[f.ei].to];
    auto const& B = *ps[edges[f.ej].to];
    auto const& C = *ps[edges[f.ek].to];
    return cross(B - A, C - A).z < 0;
  }

  auto get_vertex_indices(convex_hull_details::face const& f) const -> std::array<int, 3> {
    return std::array{
        ps[edges[f.ei].to].index,
        ps[edges[f.ej].to].index,
        ps[edges[f.ek].to].index,
    };
  };

  auto new_face(int ei, int ej, int ek) -> void {
    edges[ei].face = int(faces.size());
    edges[ej].face = int(faces.size());
    edges[ek].face = int(faces.size());
    faces.emplace_back(ei, ej, ek);
  }

  auto face_orientation(int point_i, int f) -> int {
    return this->check_orientation(
        *ps[edges[faces[f].ei].to], *ps[edges[faces[f].ej].to], *ps[edges[faces[f].ek].to],
        *ps[point_i]);
  }

  auto _build() -> void {
    for (size_t i = 0; i < ps.size(); i++) {
      ps[i].index = int(i);
    }

    // make unique first?
    std::sort(ps.begin(), ps.end());
    ps.erase(std::unique(ps.begin(), ps.end()), ps.end());

    // find tetrahedron
    size_t i = 1;
    for (; i < ps.size(); i++) {
      if (not this->check_equal(*ps[0], *ps[i])) {
        std::swap(ps[i], ps[1]);
        break;
      }
    }
    if (i == ps.size()) {
      dimension = 0;
      return;
    }
    for (++i; i < ps.size(); i++) {
      if (not this->check_collinear(*ps[0], *ps[1], *ps[i])) {
        std::swap(ps[i], ps[2]);
        break;
      }
    }
    if (i == ps.size()) {
      dimension = 1;
      return;
    }
    for (++i; i < ps.size(); i++) {
      if (this->check_orientation(*ps[0], *ps[1], *ps[2], *ps[i])) {
        std::swap(ps[i], ps[3]);
        break;
      }
    }

    // run 2d or 3d convex hull
    if (i == ps.size()) {
      dimension = 2;
      _solve_2d();
    } else {
      dimension = 3;
      _solve_3d();
    }

    // TODO merge coplanar faces if needed
  }

  auto _solve_2d() {
    auto const p2d = this->get_2d_points(cross(*ps[1] - *ps[0], *ps[2] - *ps[0]), ps);
    auto const hull2d = this->do_convex_hull(p2d);
    std::vector<int> pids(hull2d.size());
    for (size_t i = 0; i < hull2d.size(); i++) {
      pids[i] = int(std::lower_bound(p2d.begin(), p2d.end(), hull2d[i]) - p2d.begin());
    }
    edges.emplace_back(pids[1]);
    edges.emplace_back(pids[0]);
    // [0 1, 1 2, 0 2]
    for (size_t i = 2; i < hull2d.size(); i++) {
      auto const cur_eid = int(edges.size());
      edges.emplace_back(pids[i]);
      edges.emplace_back(pids[i - 1]);
      edges.emplace_back(pids[i]);
      edges.emplace_back(pids[0]);
      // make face [i, i-1, 0]
      new_face(cur_eid + 1, cur_eid - 1, cur_eid + 2);
    }
    for (size_t i = hull2d.size() - 1; i >= 2; i--) {
      // make face [i-1, i, 0]
      auto const cur_eid = int(edges.size());
      auto const outer_e = 4 * int(i) - 6;
      if (i > 2) {
        edges.emplace_back(pids[i - 1]);
        edges.emplace_back(pids[0]);
        new_face(outer_e, cur_eid - 1, cur_eid);
      } else {
        new_face(outer_e, cur_eid - 1, 0);
      }
    }
  }

  auto _solve_3d() -> void {
    // make the initial faces
    if (this->check_orientation(*ps[0], *ps[1], *ps[2], *ps[3]) < 0) {
      std::swap(ps[1], ps[2]);
    }
    for (int i = 0; i < 4; i++) {
      for (int j = i + 1; j < 4; j++) {
        edges.emplace_back(j);
        edges.emplace_back(i);
      }
    }
    // 0 1, 0 2, 0 3, 1 2, 1 3, 2 3
    //  0    1    2    3    4    5
    new_face(2 * 3 + 1, 2 * 0 + 1, 2 * 1 + 0);  // 2 1 0
    new_face(2 * 2 + 1, 2 * 0 + 0, 2 * 4 + 0);  // 3 0 1
    new_face(2 * 4 + 1, 2 * 3 + 0, 2 * 5 + 0);  // 3 1 2
    new_face(2 * 5 + 1, 2 * 1 + 1, 2 * 2 + 0);  // 3 2 0
    // edges are such that e and e^1 are reverses of each other, so
    // - each half-edge is incident to exactly one face
    // - each face is incident to exactly three half-edges

    static_cast<derived_t*>(this)->_solve_3d();

    // cleanup any removed faces
    faces.erase(
        std::remove_if(faces.begin(), faces.end(), [](auto const& f) { return f.removed; }),
        faces.end());
    for (int i = 0; i < int(faces.size()); i++) {
      edges[faces[i].ei].face = edges[faces[i].ej].face = edges[faces[i].ek].face = i;
    }
  }
};
