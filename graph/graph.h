/* Graph (AdjacencyList)
 * USAGE
 *  ???
 * STATUS
 *  untested
 */
#pragma once

#include "graph/graph_common.h"

template <typename weight_t, graph_traits traits>
  requires AdjacencyList<traits>
struct graph_base_edges<weight_t, traits, void> {
  std::vector<std::vector<graph_adj<weight_t>>> adj;
  graph_base_edges(size_t n) : adj(n) {}
  auto size() const -> int { return (int)adj.size(); }
  auto reset(size_t n) -> void {
    adj.resize(n);
    fill(adj.begin(), adj.end(), std::vector<graph_adj<weight_t>>{});
  }
  template <typename... weight_args>
    requires ValidWeightArg<weight_t, weight_args...>
  auto add_arc(int from, int to, weight_args... weight) -> void {
    adj[from].emplace_back(to, weight...);
  }
  auto get_edges() const -> std::vector<graph_edge<weight_t>> {
    std::vector<graph_edge<weight_t>> edges;
    for (int u = 0; u < size(); u++) {
      for (auto const& v : adj[u]) {
        edges.emplace_back(u, v);
      }
    }
    return edges;
  }
  auto operator[](int u) const -> std::vector<graph_edge<weight_t>> { return this->adj[u]; }
  friend auto operator<<(std::ostream& os, const graph_base_edges& g) -> std::ostream& {
    for (int u = 0; u < g.size(); u++) {
      os << u << " -> [";
      if constexpr (g.weighted) {
        for (auto [v, c] : g[u]) {
          os << " " << v << ":" << c << ", ";
        }
      } else {
        for (int v : g[u]) {
          os << " " << v << ", ";
        }
      }
      os << "]\n";
    }
    return os;
  }
};
