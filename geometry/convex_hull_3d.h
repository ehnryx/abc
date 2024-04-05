/* 3D Convex Hull (randomized Quickhull)
 * USAGE
 *  convex_hull<T>(begin(points), end(points));
 * INPUT
 *  points: iterable of point3d<T>
 * OUTPUT
 *  faces should be oriented outwards (right-hand-rule)
 *  faces should not have interior points
 * TIME
 *  expected O(NlogN)
 *  N = #points
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
struct face {
  int ei, ej, ek;  // incident edge indices
  bool removed = false;
};
struct edge {
  int to;    // destination of half-edge
  int face;  // incident face index
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
  auto check_sign(auto const&... args) { return geometry::sign(args...); };
  auto check_equal(auto const& a, auto const& b) { return a == b; };
  auto check_collinear(auto const&... args) { return collinear(args...); }
  auto check_orientation(auto const&... args) { return plane_point_orientation(args...); }
  auto do_convex_hull(auto const&... args) { return convex_hull(args...); }
};
template <std::floating_point T>
struct epsilon_base<T> {
  epsilon<T> const eps;
  auto check_sign(auto const&... args) { return geometry::sign(eps, args...); };
  auto check_equal(auto const&... args) { return geometry::equal(eps, args...); };
  auto check_collinear(auto const&... args) { return collinear(eps, args...); }
  auto check_orientation(auto const&... args) { return plane_point_orientation(eps, args...); }
  auto do_convex_hull(auto const&... args) { return convex_hull(eps, args...); }
};
}  // namespace convex_hull_details

template <typename T>
struct convex_hull_3d : convex_hull_details::epsilon_base<T> {
  static constexpr int MAGIC_SHUFFLE_NUMBER = 120;  // magic number for randomizing faces ?
  std::vector<convex_hull_details::point<T>> ps;
  std::vector<convex_hull_details::face> faces;
  std::vector<convex_hull_details::edge> edges;
  int dimension = 0;

  template <std::floating_point F = T, std::input_iterator InputIt>
  convex_hull_3d(epsilon<F> e, InputIt first, InputIt last)
      : convex_hull_details::epsilon_base<T>(e), ps(first, last) {
    _build();
  }

  template <std::input_iterator InputIt>
  convex_hull_3d(InputIt first, InputIt last) : ps(first, last) {
    _build();
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
    if (i == ps.size()) {
      dimension = 2;
      return _solve_2d();
    }

    // found tetrahedron
    dimension = 3;
    return _solve_3d();
  }

  auto _solve_2d() {
    std::vector<point<T>> p2d;
    p2d.reserve(ps.size());
    auto const normal = cross(*ps[1] - *ps[0], *ps[2] - *ps[0]);
    if (this->check_sign(normal.z)) {
      for (size_t i = 0; i < ps.size(); i++) {
        p2d.emplace_back(ps[i]->x, ps[i]->y);
      }
    } else if (this->check_sign(normal.x)) {
      for (size_t i = 0; i < ps.size(); i++) {
        p2d.emplace_back(ps[i]->y, ps[i]->z);
      }
    } else {  // (0, 1, 0)
      for (size_t i = 0; i < ps.size(); i++) {
        p2d.emplace_back(ps[i]->x, ps[i]->z);
      }
    }
    // p2d should already be sorted and uniqued
    auto const hull2d = this->do_convex_hull(p2d);
    std::vector<int> pids(hull2d.size());
    for (size_t i = 0; i < hull2d.size(); i++) {
      pids[i] = int(std::lower_bound(p2d.begin(), p2d.end(), hull2d[i]) - p2d.begin());
      if (i > 0) {
        edges.emplace_back(pids[i]);
        edges.emplace_back(pids[0]);
      }
    }
    for (int i = 2; i < int(hull2d.size()); i++) {
      auto const cur_eid = int(edges.size());
      edges.emplace_back(pids[i]);
      edges.emplace_back(pids[i - 1]);
      new_face(2 * (i - 1) + 1, 2 * (i - 2), cur_eid);
      new_face(2 * (i - 2) + 1, 2 * (i - 1), cur_eid ^ 1);
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

    // initialize points assignments
    std::vector<std::vector<int>> points_for_face(faces.size());
    std::vector<int> unprocessed = {0, 1, 2, 3};
    for (int i = 4; i < int(ps.size()); i++) {
      if (i % MAGIC_SHUFFLE_NUMBER == 0) {
        std::shuffle(unprocessed.begin(), unprocessed.end(), get_rng());
      }
      for (int f : unprocessed) {
        if (face_orientation(i, f) > 0) {
          points_for_face[f].push_back(i);
          break;
        }
      }
    }

    enum class face_type { NONE = 0, INSIDE = 1, OUTSIDE = 2 };
    std::vector<face_type> visited(faces.size());
    std::vector<int> new_edge(ps.size());  // TODO this is really bad for cache?
    while (not unprocessed.empty()) {
      int cur_f = unprocessed.back();
      unprocessed.pop_back();
      if (visited[cur_f] == face_type::INSIDE) continue;  // already removed
      if (points_for_face[cur_f].empty()) continue;

      // get the farthest point
      auto const& A = *ps[edges[faces[cur_f].ei].to];
      auto const& B = *ps[edges[faces[cur_f].ej].to];
      auto const& C = *ps[edges[faces[cur_f].ek].to];
      auto const normal = cross(B - A, C - A);
      int const cur_i = *max_element(
          points_for_face[cur_f].begin(), points_for_face[cur_f].end(),
          [&](int i, int j) { return normal.dot(*ps[i] - *ps[j]) > 0; });

      // bfs to find conflicting faces
      visited[cur_f] = face_type::INSIDE;
      std::vector<int> inside, outside, border;
      inside.push_back(cur_f);
      for (size_t bfs = 0; bfs < inside.size(); bfs++) {
        int const u = inside[bfs];
        for (int e : {faces[u].ei, faces[u].ej, faces[u].ek}) {
          int f = edges[e ^ 1].face;
          if (visited[f] == face_type::NONE) {
            if (face_orientation(cur_i, f) > 0) {
              visited[f] = face_type::INSIDE;
              inside.push_back(f);
            } else {
              visited[f] = face_type::OUTSIDE;
              outside.push_back(f);
            }
          }
          if (visited[f] == face_type::OUTSIDE) {
            new_edge[edges[e].to] = int(edges.size());
            edges.emplace_back(cur_i);
            edges.emplace_back(edges[e].to);
            border.push_back(e);
          }
        }
      }

      // create faces and push to stack
      for (int e : border) {
        unprocessed.push_back(int(faces.size()));
        new_face(new_edge[edges[e].to], new_edge[edges[e ^ 1].to] ^ 1, e);
        points_for_face.emplace_back();
      }

      // assign points to faces
      for (int iters = 0; int f : inside) {
        for (int i : points_for_face[f]) {
          if (i == cur_i) continue;
          if (++iters == MAGIC_SHUFFLE_NUMBER) {
            std::shuffle(border.begin(), border.end(), get_rng());
            iters = 0;
          }
          for (int e : border) {
            if (face_orientation(i, edges[e].face) > 0) {
              points_for_face[edges[e].face].push_back(i);
              break;
            }
          }
        }
      }

      // remove faces and clear visited
      for (int f : inside) {
        faces[f].removed = true;
        points_for_face[f].clear();
      }
      for (int f : outside) {
        visited[f] = face_type::NONE;
      }
      visited.resize(faces.size(), face_type::NONE);
    }

    // cleanup any removed faces
    faces.erase(
        std::remove_if(faces.begin(), faces.end(), [](auto const& f) { return f.removed; }),
        faces.end());

    // TODO merge coplanar faces
  }

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
};
