/** Randomized Incremental 3D Convex Hull
 * TIME
 *  expected O(NlogN), but the constant is huge
 *  N = #points
 * STATUS
 *  tested: boj/23459,21065
 */
#pragma once

#include "geometry/convex_hull_3d_base.h"

template <typename T>
struct convex_hull_3d_incremental : convex_hull_3d_base<convex_hull_3d_incremental<T>, T> {
  using base_convex_hull = convex_hull_3d_base<convex_hull_3d_incremental<T>, T>;
  using base_convex_hull::base_convex_hull;
  using base_convex_hull::face_orientation, base_convex_hull::new_face;
  using base_convex_hull::ps, base_convex_hull::edges, base_convex_hull::faces;

  auto _solve_3d() -> void {
    std::shuffle(ps.begin() + 4, ps.end(), get_rng());

    // initialize conflict graph
    std::vector<std::vector<int>> faces_for_point(ps.size());
    std::vector<std::vector<int>> points_for_face(faces.size());
    for (int i = 4; i < int(ps.size()); i++) {
      for (int f = 0; f < 4; f++) {
        if (face_orientation(i, f) > 0) {
          // points_for_face is sorted and will remain sorted
          points_for_face[f].push_back(i);
          faces_for_point[i].push_back(f);
        }
      }
    }

    // start incremental
    std::vector<int> new_edge(ps.size());  // TODO this is really bad for cache?
    for (size_t cur_i = 4; cur_i < ps.size(); cur_i++) {
      auto f_end = std::remove_if(
          faces_for_point[cur_i].begin(), faces_for_point[cur_i].end(), [&](int f) {
            if (faces[f].removed) return true;
            else {
              faces[f].removed = true;
              return false;
            }
          });
      if (f_end == faces_for_point[cur_i].begin()) continue;  // no conflicting faces

      // make new edges to cur_i
      for (auto f_it = faces_for_point[cur_i].begin(); f_it != f_end; f_it++) {
        for (auto e : this->get_edge_indices(faces[*f_it])) {
          if (faces[edges[e ^ 1].face].removed) continue;
          // found a border edge, direct new edge towards cur_i
          new_edge[edges[e].to] = int(edges.size());
          edges.emplace_back(cur_i);
          edges.emplace_back(edges[e].to);
        }
      }

      // for each border edge, create a new face and update the conflicts
      for (auto f_it = faces_for_point[cur_i].begin(); f_it != f_end; f_it++) {
        for (auto e : this->get_edge_indices(faces[*f_it])) {
          int const adj_f = edges[e ^ 1].face;
          if (faces[adj_f].removed) continue;
          int const new_f = int(faces.size());
          new_face(new_edge[edges[e].to], new_edge[edges[e ^ 1].to] ^ 1, e);
          points_for_face.emplace_back();
          std::set_union(
              points_for_face[*f_it].begin(), points_for_face[*f_it].end(),
              points_for_face[adj_f].begin(), points_for_face[adj_f].end(),
              std::back_inserter(points_for_face[new_f]));
          points_for_face[new_f].erase(
              std::remove_if(
                  points_for_face[new_f].begin(), points_for_face[new_f].end(),
                  [&](int j) { return face_orientation(j, new_f) <= 0; }),
              points_for_face[new_f].end());
          for (int j : points_for_face[new_f]) {
            faces_for_point[j].push_back(new_f);
          }
        }
        points_for_face[*f_it].clear();
      }
      faces_for_point[cur_i].clear();
    }
  }
};
