/* Lowest Common Ancestor (Binary Jumping)
 * USAGE
 * STATUS
 *  untested: kattis/tourists
 */
#pragma once

#include "graph/tree.h"

#include <bit>

template <tree_traits extra_traits = tree_traits::NONE>
struct lca_binary_jumping
    : tree_t<void, extra_traits | tree_traits::DEPTH | tree_traits::PARENT> {
  using tree_base = tree_t<void, extra_traits | tree_traits::DEPTH | tree_traits::PARENT>;
  int logn;
  std::vector<std::vector<int>> anc;
  lca_binary_jumping(int n)
      : tree_base(n), logn(std::bit_width(unsigned(n))), anc(logn, std::vector<int>(n, -1)) {}
  auto build(int r) -> void {
    tree_base::build(r, [this](int u) {
      anc[0][u] = this->parent[u];
      for (int j = 1; j < logn and anc[j - 1][u] != -1; j++) {
        anc[j][u] = anc[j - 1][anc[j - 1][u]];
      }
    });
  }
  auto lca(int a, int b) const -> int {
    if (this->depth[a] < this->depth[b]) std::swap(a, b);
    for (int j = logn - 1; j >= 0; j--) {
      if (this->depth[a] - (1 << j) >= this->depth[b]) {
        a = anc[j][a];
      }
    }
    if (a == b) return a;
    for (int j = logn - 1; j >= 0; j--) {
      if (anc[j][a] != anc[j][b]) {
        a = anc[j][a];
        b = anc[j][b];
      }
    }
    return this->parent[a];
  }
  auto distance(int a, int b) const -> int {
    return this->depth[a] + this->depth[b] - 2 * this->depth[lca(a, b)];
  }
  auto jump(int u, int len) const -> int {
    for (int j = logn - 1; j >= 0; j--) {
      if (len >> j & 1) {
        u = anc[j][u];
      }
    }
    return u;
  }
};
