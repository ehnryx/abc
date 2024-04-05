/* Half-Plane Intersection
 * USAGE
 *  input: a list of segments (half-plane to the right of them)
 *  output: polygon representing the half-plane intersection.
 *          empty if empty
 *  assumption: intersection is not unbounded
 * STATUS [bounded intersection]
 *  tested: boj/3800,3903,21048
 * STATUS [unbounded intersection]
 *  untested. probably doesn't work. don't
 */
#pragma once

#include "geometry/polygon.h"
#include "geometry/segment.h"

#include <algorithm>

namespace geometry {
template <typename T>
struct envelope_line {
  segment<T> const line;
  double first, last;
};
}  // namespace geometry

/// assumes that the intersection is bounded
template <typename T>
auto half_plane_intersection(std::vector<segment<T>> segs)
    -> polygon<typename point<T>::intersection_t> {
  std::sort(begin(segs), end(segs), [](segment<T> const& a, segment<T> const& b) {
    return point<T>::ccw_from_ref({0, -1}, [&a, &b](...) {
      return cross(a.end - a.start, b.end - a.start) < 0;
    })(a.direction(), b.direction());
  });
  segs.erase(
      std::unique(
          segs.begin(), segs.end(),
          [](segment<T> const& a, segment<T> const& b) {
            if constexpr (std::is_floating_point_v<T>) {
              return parallel(epsilon<T>{0}, a.start, a.end, b.start, b.end);
            } else {
              return parallel(a.start, a.end, b.start, b.end);
            }
          }),
      segs.end());
  std::vector<geometry::envelope_line<T>> envelope;
  envelope.reserve(segs.size());
  envelope.emplace_back(segs[0]);
  for (size_t i = 1; i < segs.size(); i++) {
    auto it = line_inter(envelope.back().line, segs[i]);
    while (envelope.size() > 1) {
      auto cur = envelope.back().line.get(it);
      // TODO eps?
      if (cur > envelope.back().first) {
        break;
      }
      envelope.pop_back();
      it = line_inter(envelope.back().line, segs[i]);
    }
    // TODO eps?
    if (envelope.back().line.direction().cross(segs[i].direction()) <= 0) {
      return {};  // right turn in a ccw-oriented hull. empty or unbounded
    }
    envelope.back().last = envelope.back().line.get(it);
    envelope.emplace_back(segs[i], segs[i].get(it));
  }
  for (int s = 0, t = (int)envelope.size() - 1; t - s > 1;) {
    auto it = line_inter(envelope[s].line, envelope[t].line);
    auto s_first = envelope[s].line.get(it);
    auto t_last = envelope[t].line.get(it);
    if (s_first > envelope[s].last) {
      s++;
    } else if (t_last < envelope[t].first) {
      t--;
    } else {
      envelope[s].first = s_first;
      envelope[t].last = t_last;
      std::vector<point<typename point<T>::intersection_t>> vertices;
      vertices.reserve(t - s + 1);
      for (int i = s; i <= t; i++) {
        vertices.push_back(envelope[i].line.get(envelope[i].first));
      }
      return polygon(std::move(vertices));
      // TODO not rotated so the first point is also minimum by operator<
    }
  }
  return {};
}
