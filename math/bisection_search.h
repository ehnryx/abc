/* Bisection Search (binary_search, but the name is taken)
 *  only continuous is supported.
 *  you can code your own binary search on indices. i believe in you!
 * STATUS
 *  tested: boj/5255
 */
#pragma once

#include <concepts>

/// Function gets evaluated on `mid = left + (right - left) / 2`
/// Return true if we should continue in [left, mid), false if (mid, right]
/// It is not required to have `left <= right`
template <std::floating_point Coordinate, typename Function>
auto bisection_search(Coordinate left, Coordinate right, Function const& f, int n_iters = 64)
    -> Coordinate {
  for (int iteration = 0; iteration < n_iters; iteration++) {
    auto const mid = left + (right - left) / 2;
    if (f(mid)) right = mid;
    else left = mid;
  }
  return (left + right) / 2;
}

/// Function gets evaluated on `mid = left + (right - left) / 2`
/// Should return true if we should continue in [left, mid], false if (mid, right]
/// It is not required to have `left <= right`
template <std::integral Index, typename Function>
auto bisection_search(Index left, Index right, Function const& f) -> Index {
  auto const search_dir = left < right ? 1 : -1;
  while (left != right) {
    auto const mid = left + (right - left) / 2;
    if (f(mid)) right = mid;
    else left = mid + search_dir;
  }
  return left;
}
