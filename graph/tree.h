/* Tree (???)
 * USAGE
 *  ???
 * STATUS
 *  untested: kattis/colorfultrees,tourists
 */
#pragma once

#include "graph/graph_utility.h"
#include "utility/traits.h"

#include <vector>

// clang-format off
MAKE_TRAITS(tree_traits,
  (PARENT, DEPTH, SUBTREE, EULER_TOUR),
);
// clang-format on

template <tree_traits>
struct tree_base_parent {
  tree_base_parent(size_t) {}
  auto set_parent(...) {}
};

template <tree_traits traits>
  requires(traits.has_any(tree_traits::PARENT))
struct tree_base_parent<traits> {
  std::vector<int> parent;
  tree_base_parent(size_t n) : parent(n, -1) {}
  auto set_parent(int cur, int par) -> void { parent[cur] = par; }
};

template <tree_traits>
struct tree_base_depth {
  tree_base_depth(size_t) {}
  auto update_depth(...) {}
};

template <tree_traits traits>
  requires(traits.has_any(tree_traits::DEPTH))
struct tree_base_depth<traits> {
  std::vector<int> depth;
  tree_base_depth(size_t n) : depth(n, 0) {}
  auto update_depth(int cur, int par) -> void { depth[cur] = depth[par] + 1; }
};

template <tree_traits>
struct tree_base_subtree {
  tree_base_subtree(size_t) {}
  auto update_subtree(...) {}
};

template <tree_traits traits>
  requires(traits.has_any(tree_traits::SUBTREE))
struct tree_base_subtree<traits> {
  std::vector<int> subtree;
  tree_base_subtree(size_t n) : subtree(n, 1) {}
  auto update_subtree(int cur, int child) -> void { subtree[cur] += subtree[child]; }
};

template <tree_traits>
struct tree_base_euler_tour {
  tree_base_euler_tour(size_t) {}
  auto update_euler_tour(...) {}
};

template <tree_traits traits>
  requires(traits.has_any(tree_traits::EULER_TOUR))
struct tree_base_euler_tour<traits> {
  std::vector<int> start, preorder, in, out;
  tree_base_euler_tour(size_t n) : start(n), in(n), out(n) { preorder.reserve(n); }
  auto is_ancestor_of(int anc, int v) const -> bool {
    return in[anc] < in[v] and out[anc] > out[v];
  }
  auto update_euler_tour(int cur, int idx) -> void {
    in[cur] = idx;
    start[cur] = int(preorder.size());
    preorder.push_back(cur);
  }
};

template <typename Builder>
constexpr bool _valid_builder_signature() {
  return requires(Builder b, int u) { b(u); };
}

template <typename... builders>
concept ValidBuilderArg =
    [] { return sizeof...(builders) <= 1 and (... && _valid_builder_signature<builders>()); }();

template <typename _weight_t = void, tree_traits traits = tree_traits::NONE>
struct tree_t : tree_base_parent<traits>,
                tree_base_depth<traits>,
                tree_base_subtree<traits>,
                tree_base_euler_tour<traits> {
  using weight_t = _weight_t;
  using adj_t = graph_adj<weight_t>;
  using edge_t = graph_edge<weight_t>;

  std::vector<std::vector<adj_t>> adj;
  int root;

  tree_t(int n)
      : tree_base_parent<traits>(n), tree_base_depth<traits>(n), tree_base_subtree<traits>(n),
        tree_base_euler_tour<traits>(n), adj(n), root(-1) {}
  auto size() const -> size_t { return adj.size(); }
  auto operator[](int i) const -> std::vector<adj_t> const& { return adj[i]; }

  template <typename... weight_args>
    requires ValidWeightArg<weight_t, weight_args...>
  auto add_edge(int u, int v, weight_args... weight) -> void {
    adj[u].emplace_back(v, weight...);
    adj[v].emplace_back(u, weight...);
  }

  template <typename... Builder>
    requires ValidBuilderArg<Builder...>
  auto build(int r, Builder&&... preorder_builder) -> void {
    root = r;
    if constexpr (traits.has_any(tree_traits::PARENT)) {
      this->parent[root] = -1;
    }
    if constexpr (traits.has_any(tree_traits::DEPTH)) {
      this->depth[root] = 0;
    }
    if constexpr (traits.has_any(tree_traits::EULER_TOUR)) {
      this->preorder.clear();
    }
    _build(r, -1, 0, preorder_builder...);
  }
  template <typename... Function>
  auto _build(int u, int par, int tour_idx, Function&... preorder_builder) -> int {
    this->update_euler_tour(u, tour_idx++);
    if constexpr (traits.has_any(tree_traits::SUBTREE)) {
      this->subtree[u] = 1;
    }
    if constexpr (sizeof...(preorder_builder) > 0) {
      (..., preorder_builder(u));
    }
    for (adj_t const& e : adj[u]) {
      if (e.to == par) continue;
      this->set_parent(e.to, u);
      this->update_depth(e.to, u);
      tour_idx = _build(e.to, u, tour_idx, preorder_builder...);
      this->update_subtree(u, e.to);
    }
    if constexpr (traits.has_any(tree_traits::EULER_TOUR)) {
      this->out[u] = tour_idx++;
    }
    return tour_idx;
  }
};
