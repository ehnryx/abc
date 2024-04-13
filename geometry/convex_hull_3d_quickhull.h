/** 3D Quickhull
 *  seems to be faster than randomized incremental in general
 * TIME
 *  unknown, but seems to be O(nlogn)
 *  N = #points
 * STATUS
 *  tested: boj/4795,5401,7684,14507
 */
#pragma once

#include "geometry/convex_hull_3d_base.h"

template <typename T>
struct convex_hull_3d_quickhull : convex_hull_3d_base<convex_hull_3d_quickhull<T>, T> {
  using base_convex_hull = convex_hull_3d_base<convex_hull_3d_quickhull<T>, T>;
  using base_convex_hull::base_convex_hull;
  using base_convex_hull::face_orientation, base_convex_hull::new_face;
  using base_convex_hull::ps, base_convex_hull::edges, base_convex_hull::faces;

  static constexpr int MAGIC_SHUFFLE_NUMBER = 120;  // magic number for randomizing faces ?
  auto _solve_3d() -> void {
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
  }
};
