/* Splay Tree
 * USAGE
 *  pray
 * TIME
 *  O(logN) per operation amortized
 *  N = |splay tree|
 * STATUS
 *  operator[]
 *    untested: cf/104941f
 *  split/merge
 *    untested: 102787a
 *  other
 *    untested: treap contest: 102787
 */
#pragma once

#include "utility/traits.h"

#include <memory>
#include <random>

// clang-format off
MAKE_TRAITS(treap_traits,
  (ORDER_STATS, PERSISTENT),
  NODE_TRAITS = ORDER_STATS,
);
// clang-format on

// tags
struct treap_index {};
struct treap_key {};

namespace treap_details {
inline auto get_rand() -> uint_fast32_t {
#if not defined(TREAP_SEED)
  static auto _rng = std::minstd_rand();
#else
  static auto _rng = std::minstd_rand(TREAP_SEED);
#endif
  return static_cast<uint_fast32_t>(_rng());
}

template <typename, treap_traits traits>
struct node_size {};

template <typename derived_t, treap_traits traits>
  requires(traits.has_all(traits.ORDER_STATS))
struct node_size<derived_t, traits> {
  uint_fast32_t size = 1;
  auto _pull_treap_size(node_size const* left, node_size const* right) -> void {
    size = 1 + (left == nullptr ? 0 : left->size) + (right == nullptr ? 0 : right->size);
  }
  auto search(uint_fast32_t& index, treap_index) -> int_fast32_t {
    auto const left = static_cast<derived_t*>(this)->left;
    auto const left_size = left == nullptr ? 0 : left->size;
    if (index < left_size) {
      return -1;
    } else if (index > left_size) {
      index -= left_size + 1;
      return 1;
    } else {
      return 0;
    }
  }
};

template <typename derived_t>
struct node_data {
  derived_t* left = nullptr;
  derived_t* right = nullptr;
  uint_fast32_t const heap_depth;
  node_data() : heap_depth(get_rand()) {}
};
}  // namespace treap_details

template <typename derived_t, typename Key_t, treap_traits traits = treap_traits::NONE>
  requires((traits & ~traits.NODE_TRAITS) == traits.NONE)
struct treap_node_base : treap_details::node_data<derived_t>,
                         treap_details::node_size<derived_t, traits> {
  using key_t = Key_t;
  key_t key;
  template <typename... Args>
  treap_node_base(key_t const& k) : key(k) {}
};

template <typename derived_t, treap_traits traits>
struct treap_node_base<derived_t, void, traits> : treap_details::node_data<derived_t>,
                                                  treap_details::node_size<derived_t, traits> {
  using key_t = void;
};

template <typename key_t, typename value_t, treap_traits traits = treap_traits::NONE>
struct treap_map_node final
    : treap_node_base<treap_map_node<key_t, value_t, traits>, key_t, traits> {
  value_t value;
  template <typename... Args>
  treap_map_node(key_t const& k, Args&&... args)
      : treap_node_base<treap_map_node<key_t, value_t, traits>, key_t, traits>(k),
        value(std::forward<Args>(args)...) {}
};

template <typename value_t, treap_traits traits>
struct treap_map_node<void, value_t, traits> final
    : treap_node_base<treap_map_node<void, value_t, traits>, void, traits> {
  value_t value;
  template <typename... Args>
  treap_map_node(Args&&... args) : value(std::forward<Args>(args)...) {}
};

template <typename key_t, treap_traits traits>
struct treap_map_node<key_t, void, traits> final
    : treap_node_base<treap_map_node<key_t, void, traits>, key_t, traits> {
  treap_map_node(key_t const& k)
      : treap_node_base<treap_map_node<key_t, void, traits>, key_t, traits>(k) {}
};

template <treap_traits traits>
struct treap_map_node<void, void, traits> final
    : treap_node_base<treap_map_node<void, void, traits>, void, traits> {};

template <typename Node_t, treap_traits traits, typename Alloc>
  requires(requires(Alloc alloc, size_t s) {
    { alloc.allocate(s) } -> std::same_as<Node_t*>;
  })
struct treap : Alloc {
  static_assert(not traits.has_any(traits.PERSISTENT), "not implemented");
  using node_t = Node_t;
  using key_t = typename node_t::key_t;
  static constexpr bool has_pull =
      requires(node_t* n) { n->_pull_treap_size(n, n); } or requires(node_t* n) { n->pull(); };

  node_t* root = nullptr;

  treap() {}
  treap(node_t* r) : root(r) {}
  treap(treap&& other) : root(other.root) { other.root = nullptr; }
  ~treap() { erase_tree(root); }

  treap& operator=(treap&& other) {
    if (this != &other) {
      erase_tree(root);
      root = other.root;
      other.root = nullptr;
    }
    return *this;
  }

  auto dislodge() -> treap { return treap(std::move(*this)); }
  auto erase_tree(node_t* u) -> void {
    if (u == nullptr) return;
    erase_tree(u->left);
    erase_tree(u->right);
    delete_node(u);
  }
  template <typename... Args>
  auto new_node(Args&&... args) -> node_t* {
    return std::construct_at(Alloc::allocate(1), std::forward<Args>(args)...);
  }
  auto delete_node(node_t* u) -> void {
    std::destroy_at(u);
    Alloc::deallocate(u, 1);
  }

  auto pull(node_t* u) -> void {
    if constexpr (traits.has_any(traits.ORDER_STATS)) {
      u->_pull_treap_size(u->left, u->right);
    }
    if constexpr (requires { u->pull(); }) {
      u->pull();
    }
  }

  auto push(node_t* u) -> void {
    if constexpr (requires { u->push(); }) {
      u->push();
    }
  }

  template <typename... Args>
  auto emplace_at(uint_fast32_t index, Args&&... args) -> node_t* {
    auto [left, right] = _split(index, treap_index{});
    node_t* x = new_node(std::forward<Args>(args)...);
    root = _merge_with_mid(left, x, right);
    return x;
  }

  auto split_at(uint_fast32_t index) -> treap
    requires(traits.has_any(traits.ORDER_STATS))
  {
    auto [left, right] = _split(index, treap_index{});
    root = right;
    return treap(left);
  }

  template <std::integral... Indices>
    requires(sizeof...(Indices) >= 2)
  auto split_at(Indices... indices) -> auto
    requires(traits.has_any(traits.ORDER_STATS))
  {
    auto inds = std::array{static_cast<uint_fast32_t>(indices)...};
    std::adjacent_difference(inds.begin(), inds.end(), inds.begin());
    return _split_at_multi(inds, std::make_index_sequence<inds.size()>());
  }
  template <size_t N, size_t... Is>
  auto _split_at_multi(std::array<uint_fast32_t, N> indices, std::index_sequence<Is...>)
      -> auto {
    return std::tuple_cat(std::array{split_at(indices[Is])...});
  }

  auto append(treap&& other) -> treap& {
    root = _merge(root, other.root);
    other.root = nullptr;
    return *this;
  }

  template <typename... Args>
  auto _split(Args&&... args) -> std::pair<node_t*, node_t*> {
    node_t* left = nullptr;
    node_t* right = nullptr;
    _split_rec(root, left, right, std::forward<Args>(args)...);
    return std::pair(left, right);
  }

  template <typename... Args>
  auto _split_rec(node_t* cur, node_t*& left, node_t*& right, Args&&... args) -> void {
    if (cur == nullptr) {
      left = right = nullptr;
      return;
    }
    push(cur);
    switch (cur->search(args...)) {
      case -1:
      case 0:
        _split_rec(cur->left, left, cur->left, std::forward<Args>(args)...);
        right = cur;
        break;
      case 1:
        _split_rec(cur->right, cur->right, right, std::forward<Args>(args)...);
        left = cur;
        break;
    }
    pull(cur);
  }

  auto _merge_with_mid(node_t* left, node_t* mid, node_t* right) -> node_t* {
    return _merge(_merge(left, mid), right);
  }

  auto _merge(node_t* left, node_t* right) -> node_t* {
    if (right == nullptr) return left;
    if (left == nullptr) return right;
    return _merge_rec(left, right);
  }

  auto _merge_rec(node_t* left, node_t* right) -> node_t* {
    if (left->heap_depth < right->heap_depth) {
      push(left);
      left->right = left->right == nullptr ? right : _merge_rec(left->right, right);
      pull(left);
      return left;
    } else {
      push(right);
      right->left = right->left == nullptr ? left : _merge_rec(left, right->left);
      pull(right);
      return right;
    }
  }
};

template <
    template <typename> typename AllocT, typename key_t, typename value_t,
    treap_traits traits = treap_traits::NONE>
using make_treap_map_allocator =
    AllocT<treap_map_node<key_t, value_t, traits & traits.NODE_TRAITS>>;

template <
    typename key_t, typename value_t, treap_traits traits = treap_traits::NONE,
    typename Alloc = std::allocator<treap_map_node<key_t, value_t, traits>>>
using treap_map =
    treap<treap_map_node<key_t, value_t, traits & traits.NODE_TRAITS>, traits, Alloc>;
