/* Splay Tree
 * USAGE
 *  wrapper around splay tree nodes
 * STATUS
 *  tested: cf/104941f; boj/15010
 */
#pragma once

#include "data_structures/splay_forest.h"

template <typename SplayForest>
struct splay_tree {
  using node_t = SplayForest::node_t;
  using pointer_t = SplayForest::pointer_t;

  std::shared_ptr<SplayForest> forest;
  pointer_t root = {0};

  splay_tree(std::shared_ptr<SplayForest> const& f) : forest(f) {}
  splay_tree(std::shared_ptr<SplayForest> const& f, pointer_t r) : forest(f), root(r) {}

  splay_tree(splay_tree&& o) : forest(o.forest), root(o.root) { o.root = {0}; }
  splay_tree& operator=(splay_tree&& o) {
    forest = o.forest;
    root = o.root;
    o.root = {0};
    return *this;
  }

  splay_tree(splay_tree const& o) = delete;
  splay_tree& operator=(splay_tree const& o) = delete;

  auto empty() const -> bool { return root == 0; }
  auto size() const -> int
    requires(requires(node_t nd) { nd.size; })
  {
    return forest->get(root).size;
  }
  auto operator[](pointer_t x) -> node_t& { return forest->get(x); }
  auto operator->() -> node_t* { return &forest->get(root); }
  auto rank(pointer_t x) -> int { return forest->get(splay(x).left).size; }
  auto splay(pointer_t new_root) -> node_t& {
    root = forest->splay(new_root);
    return forest->get(root);
  }
  auto data() -> node_t* { return forest->data; }

  template <typename... Args>
  auto try_emplace(Args&&... args) -> pointer_t {
    if (root == 0) {
      root = forest->new_node(std::forward<Args>(args)...);
    } else {
      root = forest
                 ->template _search<
                     search_params::FIND | search_params::BY_KEY | search_params::EMPLACE>(
                     root, std::forward<Args>(args)...)
                 .first;
    }
    return root;
  }
  template <typename... Args>
  auto operator[](Args&&... args) -> node_t& {
    try_emplace(std::forward<Args>(args)...);
    return forest->get(root);
  }

  template <search_params params, typename... Args>
  auto erase(Args... args) -> pointer_t {
    auto found = pointer_t{0};
    if (root != 0) {
      tie(found, root) = forest->template _search<params | params.MAKE_ROOT>(root, args...);
      root = forest->_erase_root(found);
    }
    return found;
  }
  auto erase_root() -> void {
    if (root != 0) {
      root = forest->_erase_root(root);
    }
  }

  template <typename... Args>
  auto new_node(Args&&... args) -> pointer_t {
    return forest->new_node(std::forward<Args>(args)...);
  }
  template <typename... Args>
  auto new_node_at(pointer_t at, Args&&... args) -> pointer_t {
    return forest->new_node_at(at, std::forward<Args>(args)...);
  }
  template <search_params params, typename... Args>
  auto insert(pointer_t add, Args... args) -> pointer_t {
    if (root == 0) {
      root = add;
    } else {
      root = forest->template _search<params | params.INSERT>(root, add, args...).first;
    }
    return root;
  }

  template <typename... Args>
  auto emplace_back(Args&&... args) -> pointer_t {
    auto const add = forest->new_node(std::forward<Args>(args)...);
    if (root != 0) {
      forest->get(add).left = root;
      forest->get(root).parent = add;
      forest->pull(add);
    }
    return root = add;
  }
  template <typename... Args>
  auto emplace_front(Args&&... args) -> pointer_t {
    auto const add = forest->new_node(std::forward<Args>(args)...);
    if (root != 0) {
      forest->get(add).right = root;
      forest->get(root).parent = add;
      forest->pull(add);
    }
    return root = add;
  }

  auto back() -> node_t& {
    if (root != 0) root = forest->_splay(forest->_rightmost(root));
    return forest->get(root);
  }
  auto front() -> node_t& {
    if (root != 0) root = forest->_splay(forest->_leftmost(root));
    return *forest->get(root);
  }

  template <search_params params, typename... Args>
  auto search(Args... args) -> pointer_t {
    pointer_t found = {0};
    if (root != 0) {
      std::tie(found, root) = forest->template _search<params>(root, args...);
    }
    return found;
  }

  /// returns a splay tree for the part after the split
  template <search_params params, typename... Args>
  auto split(Args... args) -> splay_tree {
    if (root == 0) return splay_tree(forest);
    auto const [before, after] = forest->template _split<params>(root, args...);
    root = before;
    return splay_tree(forest, after);
  }

  /// returns result for easy chaining
  auto append(splay_tree&& other) -> splay_tree& {
    if (root == 0) root = other.root;
    else if (other.root != 0) root = forest->_append(root, other.root);
    other.root = {0};  // invalidate other
    return *this;
  }

  /// visit (TODO add iterators)
  template <typename Function>
  auto for_each(Function&& f) -> void {
    forest->visit(root, std::move(f));
  }
};

template <typename key_t, splay_traits traits = splay_traits::NONE>
struct splay_set : splay_node_base<splay_set<key_t, traits>, key_t, traits> {
  using splay_node_base<splay_set<key_t, traits>, key_t, traits>::splay_node_base;
};

template <typename node_t, typename Alloc = std::allocator<node_t>>
auto make_splay_tree(int n) -> splay_tree<splay_forest<node_t, Alloc>> {
  return splay_tree(std::make_shared<splay_forest<node_t, Alloc>>(n));
}
