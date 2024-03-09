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

namespace treap_details {
inline auto get_rand() -> uint_fast32_t {
  static std::minstd_rand _rng;
  return static_cast<uint_fast32_t>(_rng());
}

struct node_size_tag {};

template <typename, treap_traits traits>
struct node_size {};

template <typename derived_t, treap_traits traits>
  requires(traits.has_all(traits.ORDER_STATS))
struct node_size<derived_t, traits> {
  uint_fast32_t size = 1;
  auto _pull_treap_size(node_size const* left, node_size const* right) -> void {
    size = 1 + (left == nullptr ? 0 : left->size) + (right == nullptr ? 0 : right->size);
  }
  template <typename Tag>
    requires(std::is_same_v<Tag, node_size_tag>)
  auto search_direction(uint_fast32_t& index) const -> int_fast32_t {
    auto const left = static_cast<derived_t const*>(this)->left;
    if (left != nullptr) {
      if (index < left->size) return -1;
      index -= left->size;
    }
    if (index != 0) {
      index -= 1;
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

template <
    typename derived_t, typename Key_t, typename Value_t,
    treap_traits traits = treap_traits::NONE>
  requires((traits & ~traits.NODE_TRAITS) == traits.NONE)
struct treap_node_base : treap_details::node_data<derived_t>,
                         treap_details::node_size<derived_t, traits> {
  using key_t = Key_t;
  using value_t = Value_t;
  key_t key;
  value_t value;
  template <typename... Args>
  treap_node_base(key_t const& k, Args&&... args)
      : key(k), value(std::forward<Args>(args)...) {}
};

template <typename derived_t, typename Key_t, treap_traits traits>
  requires(not std::is_void_v<Key_t>)
struct treap_node_base<derived_t, Key_t, void, traits>
    : treap_details::node_data<derived_t>, treap_details::node_size<derived_t, traits> {
  using key_t = Key_t;
  using value_t = void;
  key_t key;
  treap_node_base(key_t const& k) : key(k) {}
};

template <typename derived_t, typename Value_t, treap_traits traits>
  requires(not std::is_void_v<Value_t>)
struct treap_node_base<derived_t, void, Value_t, traits>
    : treap_details::node_data<derived_t>, treap_details::node_size<derived_t, traits> {
  using key_t = void;
  using value_t = Value_t;
  value_t value;
  template <typename... Args>
  treap_node_base(Args&&... args) : value(std::forward<Args>(args)...) {}
};

template <typename derived_t, treap_traits traits>
struct treap_node_base<derived_t, void, void, traits>
    : treap_details::node_data<derived_t>, treap_details::node_size<derived_t, traits> {
  using key_t = void;
  using value_t = void;
  treap_node_base() : treap_details::node_data<derived_t>() {}
};

template <typename key_t, typename value_t, treap_traits traits = treap_traits::NONE>
struct treap_node final
    : treap_node_base<treap_node<key_t, value_t, traits>, key_t, value_t, traits> {
  using treap_node_base<
      treap_node<key_t, value_t, traits>, key_t, value_t, traits>::treap_node_base;
};

template <typename Node_t, treap_traits traits, typename Alloc>
  requires(requires(Alloc alloc, size_t s) {
    { alloc.allocate(s) } -> std::same_as<Node_t*>;
  })
struct treap : Alloc {
  static_assert(not traits.has_any(traits.PERSISTENT), "not implemented");
  using node_t = Node_t;
  using key_t = typename node_t::key_t;
  using value_t = typename node_t::value_t;
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
    auto [left, right] = _split_before(index);
    node_t* x = new_node(std::forward<Args>(args)...);
    root = _merge_middle_node(left, x, right);
    return x;
  }

  auto split_before(uint_fast32_t index) -> treap
    requires(traits.has_any(traits.ORDER_STATS))
  {
    auto [left, right] = _split_before(index);
    root = right;
    return treap(left);
  }

  template <std::integral... Indices>
    requires(sizeof...(Indices) >= 2)
  auto split_before(Indices... indices) -> auto
    requires(traits.has_any(traits.ORDER_STATS))
  {
    auto inds = std::array{static_cast<uint_fast32_t>(indices)...};
    std::adjacent_difference(inds.begin(), inds.end(), inds.begin());
    return _split_before_multi(inds, std::make_index_sequence<inds.size()>());
  }

  auto append(treap&& other) -> treap& {
    root = _merge(root, other.root);
    other.root = nullptr;
    return *this;
  }

  auto _merge_middle_node(node_t* left, node_t* single, node_t* right) -> node_t* {
    return _merge(_merge(left, single), right);
  }

  template <size_t N, size_t... Is>
  auto _split_before_multi(std::array<uint_fast32_t, N> indices, std::index_sequence<Is...>)
      -> auto {
    return std::tuple_cat(std::array{split_before(indices[Is])...});
  }

  auto _split_before(uint_fast32_t index) -> std::pair<node_t*, node_t*> {
    node_t* left = nullptr;
    node_t* right = nullptr;
    if (root == nullptr or index == root->size) {
      left = root;
    } else {
      _split_before(root, index, left, right);
    }
    return std::pair(left, right);
  }

  auto _split_before(node_t* cur, uint_fast32_t index, node_t*& left, node_t*& right) -> void {
    push(cur);
    auto const left_size = cur->left == nullptr ? 0 : cur->left->size;
    if (index < left_size) {
      _split_before(cur->left, index, left, cur->left);
      right = cur;
    } else if (left_size < index) {
      _split_before(cur->right, index - left_size - 1, cur->right, right);
      left = cur;
    } else {
      left = cur->left;
      right = cur;
      cur->left = nullptr;
    }
    pull(cur);
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

template <template <typename> typename allocator_template>
struct treap_allocator {};

template <
    template <typename> typename AllocT, typename key_t, typename value_t,
    treap_traits traits = treap_traits::NONE>
using make_treap_allocator = AllocT<treap_node<key_t, value_t, traits & traits.NODE_TRAITS>>;

template <
    typename key_t, typename value_t, treap_traits traits = treap_traits::NONE,
    typename Alloc = std::allocator<treap_node<key_t, value_t, traits>>>
using treap_map = treap<treap_node<key_t, value_t, traits & traits.NODE_TRAITS>, traits, Alloc>;
