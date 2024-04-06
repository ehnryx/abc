/** Disjoint Set Union / Union-Find
 * source: https://github.com/chenvictor/acm/blob/master/ds/DSU.cpp
 * does union by size, where size is stored in the root
 * USAGE
 *  find(x) -> int; gives the root of the group containing x
 *  size(x) -> int; gives the size of the group containing x
 *  link(x) -> bool; link x and y, returns true if new link
 *  size() -> int; gives the number of groups
 * STATUS
 *  tested: boj/3108
 */
#pragma once

#include <cstdint>
#include <vector>

struct union_find {
  std::vector<int> r;
  int_fast32_t _size;
  union_find(size_t n) : r(n, -1), _size(n) {}
  auto operator[](int x) -> int { return find(x); }
  auto find(int x) -> int { return r[x] < 0 ? x : r[x] = find(r[x]); }
  auto size(int x) -> int { return -r[find(x)]; }
  /// returns true if the link is new
  auto link(int x, int y) -> bool {
    if ((x = find(x)) == (y = find(y))) return false;
    if (r[x] < r[y]) std::swap(x, y);
    r[y] += r[x];
    r[x] = y;
    _size -= 1;
    return true;
  }
  auto size() -> int { return (int)_size; }
};

using DSU = union_find;
