/* Dinic (Max-Flow Min-Cut)
 * USAGE
 *  dinic<T> graph(n); // n = number of vertices
 *  graph.add_edge(a, b, c, bidirectional?);
 *  T flow = graph.flow(source, sink);
 * MEMBERS
 *  add_edge(a, b, c, bidirectional?); adds an edge a -> b with capacity c.
 *    bidirectional? is a boolean specifying whether the edge is undirected.
 *    default is false
 *  flow(source, sink); returns max flow from source to sink.
 *  clear_flow(); clears the flow in the graph (untested)
 * TIME
 *  O(EV^2) in general
 *  O(E min(V^{2/3}, sqrtE)) for unit caps
 *  O(EsqrtV) for bipartite matchings (bipartite + unit caps)
 *  V = #vertices, E = #edges
 * STATUS
 *  tested: kattis/maxflow,hubtown,risk,sandart, cf/100199b
 *          boj/11226(sandart),11376
 */
#pragma once

#include <limits>
#include <queue>
#include <stdexcept>
#include <vector>

template <typename T>
struct dinic {
  using sum_t = std::conditional_t<(std::is_integral_v<T> and sizeof(T) < 8), long long, T>;
  struct edge {
    int to, rev;
    T cap, flow;
    edge(int t, int r, T const& c, T const& f) : to(t), rev(r), cap(c), flow(f) {}
  };
  std::vector<std::vector<edge>> adj;
  std::vector<int> layer, cur_edge;
  dinic(size_t n) : adj(n), layer(n), cur_edge(n) {}

  auto size() const -> int { return (int)adj.size(); }
  auto operator[](int i) const -> std::vector<edge> const& { return adj[i]; }

  auto add_edge(int a, int b, T const& c = 1, bool bidirectional = false) -> void {
    adj[a].emplace_back(b, (int)adj[b].size(), c, 0);
    adj[b].emplace_back(a, (int)adj[a].size() - 1, bidirectional ? c : 0, 0);
  }
  auto left_of_min_cut(int u) const -> bool { return layer[u] != -1; }
  auto clear_flow() -> void {
    for (std::vector<edge>& adjacency : adj) {
      for (edge& e : adjacency) {
        e.flow = 0;
      }
    }
  }

  auto bfs(int s, int t) -> bool {
    std::fill(begin(layer), end(layer), -1);
    std::fill(begin(cur_edge), end(cur_edge), 0);
    std::queue<int> todo;
    layer[s] = 0;
    todo.push(s);
    while (!empty(todo) && layer[t] == -1) {
      int u = todo.front();
      todo.pop();
      for (edge const& e : adj[u]) {
        if (layer[e.to] == -1 && e.flow < e.cap) {
          layer[e.to] = layer[u] + 1;
          todo.push(e.to);
        }
      }
    }
    return layer[t] != -1;
  }

  auto dfs(int u, int t, T f) -> T {
    if (u == t || f == 0) return f;
    T res = 0;
    for (int i = cur_edge[u]; i < (int)adj[u].size() && f != 0; i++) {
      cur_edge[u] = i;  // save current edge
      if (layer[u] + 1 != layer[adj[u][i].to]) continue;
      if (T cur_flow = dfs(adj[u][i].to, t, std::min(f, adj[u][i].cap - adj[u][i].flow))) {
        adj[u][i].flow += cur_flow;
        adj[adj[u][i].to][adj[u][i].rev].flow -= cur_flow;
        res += cur_flow;
        f -= cur_flow;
      }
    }
    return res;
  }

  auto flow(int source, int sink, int max_iters = std::numeric_limits<int>::max()) -> sum_t {
    if constexpr (!std::is_integral_v<T>) {
      if (max_iters == std::numeric_limits<int>::max()) {
        throw std::invalid_argument("Must provide `max_iters` if the type is not integral");
      }
    }
    T inf = std::numeric_limits<T>::max();
    sum_t res = 0;
    for (int i = 0; i < max_iters && bfs(source, sink); i++) {
      while (T cur_flow = dfs(source, sink, inf)) {
        res += cur_flow;
      }
    }
    return res;
  }
};
