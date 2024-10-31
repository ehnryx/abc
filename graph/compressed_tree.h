/* Compressed Tree (using some LCA)
 * USAGE
 * STATUS
 *  untested: kattis/colorfultrees
 */
#pragma once

#include "graph/graph_utility.h"

#include <map>
#include <stdexcept>
#include <vector>

template <typename T>
concept HasLCA = requires(T const& t, int a, int b) {
  t.lca(a, b);
  t.distance(a, b);
};

template <typename T>
concept HasPreorder = requires(T const& t, int u) { t.in[u]; };

struct compressed_tree {
  std::map<int, int> remap;
  std::vector<int> original;
  std::vector<bool> given;  // original subset
  std::vector<std::vector<graph_adj<int>>> adj;
  std::vector<graph_adj<int>> parent;

  template <typename T>
    requires(HasLCA<T> and HasPreorder<T>)
  compressed_tree(T const& tree, const std::vector<int>& subset) {
    if (subset.empty()) {
      throw std::invalid_argument("vertices of compressed tree must not be empty");
    }
    std::vector<std::pair<int, int>> order;
    for (int v : subset) {
      order.emplace_back(tree.in[v], v);
    }
    std::sort(begin(order), end(order));
    for (size_t i = 1; i < subset.size(); i++) {
      int lca = tree.lca(order[i].second, order[i - 1].second);
      order.emplace_back(tree.in[lca], lca);
    }
    std::sort(begin(order), end(order));

    int n = int(unique(begin(order), end(order)) - begin(order));
    original.resize(n);
    adj.resize(n);
    parent.reserve(n);
    parent.emplace_back(-1, 0);
    for (int i = 0; i < n; i++) {
      original[i] = order[i].second;
      remap[original[i]] = i;
      if (i > 0) {
        int lca = tree.lca(original[i], original[i - 1]);
        int dist = tree.distance(original[i], lca);
        parent.emplace_back(remap[lca], dist);
        adj[parent[i].to].emplace_back(i, dist);
      }
    }
    given.resize(n);
    for (int v : subset) {
      given[remap[v]] = true;
    }
  }

  auto operator[](int i) const -> std::vector<graph_adj<int>> const& { return adj[i]; }
  auto size() const -> int { return int(adj.size()); }
};
