/* Graph (???)
 * USAGE
 *  graph_list<edge_t> graph(n); // n = number of vertices
 *  graph.add_edge(a, b, weight);
 * STATUS
 *  untested
 */
#pragma once

#include <vector>

struct graph_traversal {
  std::vector<int> parent;
  graph_traversal(int n) : parent(n, -1) {}
  auto get_path(int to) const -> std::vector<int> {
    std::vector<int> path;
    if (parent[to] != -1) {
      while (true) {
        path.push_back(to);
        if (to == parent[to]) break;
        to = parent[to];
      }
      reverse(path.begin(), path.end());
    }
    return path;
  }
  auto get_parents() const -> std::vector<int> { return parent; }
  auto visited(int u) const -> bool { return parent[u] != -1; }
};
