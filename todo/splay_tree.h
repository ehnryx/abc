/* Splay Tree
 * USAGE
 *  wrapper around splay tree nodes
 * STATUS
 *  tested: cf/104941f
 */
#pragma once

#include "todo/splay_forest.h"

template <typename SplayForest>
struct splay_tree {
  using node_t = SplayForest::node_t;
  using pointer_t = SplayForest::pointer_t;

  std::shared_ptr<SplayForest> forest;
  pointer_t _root = {0};
  auto root() -> node_t& { return forest->get(_root); }
  auto root() const -> node_t const& { return forest->get(_root); }
  auto nil() -> node_t& { return forest->get({0}); }
  auto nil() const -> node_t const& { return forest->get({0}); }

  splay_tree(std::shared_ptr<SplayForest> const& f) : forest(f) {}
  splay_tree(std::shared_ptr<SplayForest> const& f, pointer_t r) : forest(f), _root(r) {}

  splay_tree(splay_tree&& o) : forest(o.forest), _root(o._root) { o._root = {0}; }
  splay_tree& operator=(splay_tree&& o) {
    forest = o.forest;
    _root = o._root;
    o._root = {0};
    return *this;
  }

  splay_tree(splay_tree const& o) = delete;
  splay_tree& operator=(splay_tree const& o) = delete;

  auto empty() const -> bool { return _root == 0; }
  auto size() const -> int
    requires(requires(node_t nd) { nd.size; })
  {
    return root().size;
  }

  template <typename... Args>
  auto try_emplace(Args&&... args) -> void {
    if (_root == 0) {
      _root = forest->new_node(std::forward<Args>(args)...);
    } else {
      _root = forest
                  ->template _search<
                      search_params::FIND | search_params::BY_KEY | search_params::EMPLACE>(
                      _root, std::forward<Args>(args)...)
                  .first;
    }
  }
  template <typename... Args>
  auto operator[](Args&&... args) -> node_t& {
    try_emplace(std::forward<Args>(args)...);
    return root();
  }

  template <search_params params, typename... Args>
  auto erase(Args&&... args) -> void {
    if (_root != 0) {
      _root = forest->template find_erase<params>(_root, std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  auto emplace_back(Args&&... args) -> void {
    auto const add = forest->new_node(std::forward<Args>(args)...);
    if (_root != 0) {
      forest->get(add).left = _root;
      forest->get(_root).parent = add;
      forest->pull(add);
    }
    _root = add;
  }
  template <typename... Args>
  auto emplace_front(Args&&... args) -> void {
    auto const add = forest->new_node(std::forward<Args>(args)...);
    if (_root != 0) {
      forest->get(add).right = _root;
      forest->get(_root).parent = add;
      forest->pull(add);
    }
    _root = add;
  }

  auto back() -> node_t& {
    if (_root != 0) _root = forest->_splay(forest->_rightmost(_root));
    return root();
  }
  auto front() -> node_t& {
    if (_root != 0) _root = forest->_splay(forest->_leftmost(_root));
    return root();
  }

  /// returns a splay tree for the part after the split
  template <search_params params, typename... Args>
  auto split(Args&&... args) -> splay_tree {
    if (_root == 0) return splay_tree(forest);
    auto const [before, after] =
        forest->template _split<params>(_root, std::forward<Args>(args)...);
    _root = before;
    return splay_tree(forest, after);
  }

  template <search_params params, typename... Args>
  auto search(Args&&... args) -> node_t& {
    pointer_t found = {0};
    if (_root != 0) {
      std::tie(found, _root) =
          forest->template _search<params>(_root, std::forward<Args>(args)...);
    }
    return forest->get(found);
  }

  /// returns result for easy chaining
  auto append(splay_tree&& other) -> splay_tree& {
    if (_root == 0) _root = other._root;
    else if (other._root != 0) _root = forest->_append(_root, other._root);
    other._root = {0};  // invalidate other
    return *this;
  }

  /// visit (TODO add iterators)
  template <typename Function>
  auto for_each(Function&& f) -> void {
    forest->visit(_root, std::move(f));
  }
};

template <typename node_t, typename Alloc = std::allocator<node_t>>
auto make_splay_tree(int n) -> splay_tree<splay_forest<node_t, Alloc>> {
  return splay_tree(std::make_shared<splay_forest<node_t, Alloc>>(n));
}
