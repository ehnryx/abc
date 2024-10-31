/* Max Weight Closure
 * A closure of a directed graph is a subgraph with no outgoing edges.
 * USAGE
 *  max_closure<flow, T> graph(weights); // vector of weights
 *  graph.add_edge(a, b); // adds an edge a -> b
 *  T weight = graph.solve();
 *  vector<int> closure = graph.get_closure();
 * TIME
 *  O(flow)
 * STATUS
 *  tested: kattis/openpitmining
 */
#pragma once

#include "graph/dinic.h"

#include <limits>
#include <vector>

template <typename T>
struct max_closure : dinic<T> {
  int n;
  max_closure(std::vector<T> const& weights)
      : dinic<T>((int)size(weights) + 2), n((int)size(weights)) {
    for (int i = 0; i < n; i++) {
      if (weights[i] > 0) {
        dinic<T>::add_edge(n, i, weights[i]);
      } else if (weights[i] < 0) {
        dinic<T>::add_edge(i, n + 1, -weights[i]);
      }
    }
  }
  void add_edge(int a, int b) { dinic<T>::add_edge(a, b, std::numeric_limits<T>::max()); }
  auto solve() -> std::vector<int> {
    dinic<T>::flow(n, n + 1);
    std::vector<int> res;
    for (int i = 0; i < n; i++) {
      if (dinic<T>::left_of_min_cut(i)) {
        res.push_back(i);
      }
    }
    return res;
  }
};
