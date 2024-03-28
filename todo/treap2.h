/* Splay Tree
 * USAGE
 *  pray
 * TIME
 *  O(logN) per operation amortized
 *  N = |splay tree|
 * STATUS
 *  operator[]
 *    untested: cf/104941f
 *  split/join
 *    untested: 102787a
 *  other
 *    untested: treap contest: 102787
 */
#pragma once

#include "todo/binary_search_traits.h"
#include "utility/traits.h"

#include <memory>
#include <random>
#include <tuple>

// clang-format off
MAKE_TRAITS(treap_traits,
  (ORDER_STATS, PARENT_PTRS),
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

template <typename, treap_traits>
struct node_size {};

template <typename derived_t, treap_traits traits>
  requires(traits.has_all(traits.ORDER_STATS))
struct node_size<derived_t, traits> {
  uint_fast32_t size = 1;
  auto _pull_treap_size(node_size const* left, node_size const* right) -> void {
    size = 1 + (left == nullptr ? 0 : left->size) + (right == nullptr ? 0 : right->size);
  }
  template <typename index_t>
  auto search(index_t& index, treap_index) -> int_fast32_t {
    auto const left = static_cast<derived_t*>(this)->left;
    auto const left_size = static_cast<index_t>(left == nullptr ? 0 : left->size);
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

template <typename, treap_traits>
struct parent_pointer {};

template <typename derived_t, treap_traits traits>
  requires(traits.has_all(traits.PARENT_PTRS))
struct parent_pointer<derived_t, traits> {
  derived_t* parent = nullptr;
  void _set_parent(derived_t* p) { parent = p; }
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
struct treap_node_base : treap_details::parent_pointer<derived_t, traits>,
                         treap_details::node_data<derived_t>,
                         treap_details::node_size<derived_t, traits> {
  using key_t = Key_t;
  key_t key;
  template <typename... Args>
  treap_node_base(key_t const& k) : key(k) {}
};

template <typename derived_t, treap_traits traits>
struct treap_node_base<derived_t, void, traits>
    : treap_details::parent_pointer<derived_t, traits>,
      treap_details::node_data<derived_t>,
      treap_details::node_size<derived_t, traits> {
  using key_t = void;
};

template <typename key_t, typename value_t, treap_traits traits = treap_traits::NONE>
struct treap_map_node final
    : treap_node_base<treap_map_node<key_t, value_t, traits>, key_t, traits> {
  value_t value;
  treap_map_node(key_t const& k)
      : treap_node_base<treap_map_node<key_t, value_t, traits>, key_t, traits>(k) {}
  template <typename... Args>
  treap_map_node(key_t const& k, Args&&... args)
      : treap_node_base<treap_map_node<key_t, value_t, traits>, key_t, traits>(k),
        value(std::forward<Args>(args)...) {}
  value_t& operator*() { return value; }
};

template <typename value_t, treap_traits traits>
struct treap_map_node<void, value_t, traits> final
    : treap_node_base<treap_map_node<void, value_t, traits>, void, traits> {
  value_t value;
  treap_map_node() = default;
  template <typename... Args>
  treap_map_node(Args&&... args) : value(std::forward<Args>(args)...) {}
  value_t& operator*() { return value; }
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

template <typename Node_t, typename Alloc = std::allocator<Node_t>>
  requires(requires(Alloc alloc, size_t s) {
    { alloc.allocate(s) } -> std::same_as<Node_t*>;
  })
struct treap : Alloc {
  using node_t = Node_t;
  using key_t = typename node_t::key_t;

  node_t* root = nullptr;

  treap() {}
  treap(node_t* r) : root(r) {}
  treap(treap&& other) : root(other.root) { other.root = nullptr; }
  ~treap() { _erase_tree(root); }

  /// assumes other is not the same as this
  treap& operator=(treap&& other) {
    _erase_tree(root);
    root = other.root;
    other.root = nullptr;
    return *this;
  }

  auto clear() -> void { _erase_tree(root); }
  auto _erase_tree(node_t* u) -> void {
    if (u == nullptr) return;
    _erase_tree(u->left);
    _erase_tree(u->right);
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
    if constexpr (requires { u->_pull_treap_size(u->left, u->right); }) {
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

  auto set_parent(node_t* u, node_t* p) -> void {
    if constexpr (requires { u->_set_parent(p); }) {
      if (u != nullptr) u->_set_parent(p);
    }
  }

  template <typename Function>
  auto visit(Function&& f) -> void {
    _visit_rec(root, std::forward<Function>(f));
  }

  template <typename Function>
  auto _visit_rec(node_t* u, Function&& f) -> void {
    if (u == nullptr) return;
    _visit_rec(u->left, f);
    f(*u);
    _visit_rec(u->right, std::forward<Function>(f));
  }

  template <typename Key_t>
  auto operator[](Key_t const& key) -> decltype(auto)
    requires(requires(node_t u) { *u; })
  {
    node_t* u = _try_emplace(key);
    return **u;
  }

  template <typename... Args>
  auto emplace_back(Args&&... args) -> node_t* {
    node_t* add = new_node(std::forward<Args>(args)...);
    return _push_back(add);
  }

  template <search_params params = search_params::NONE, typename... Args>
  auto split(Args&&... args) -> treap {
    auto [left, right] = _split<params>(std::forward<Args>(args)...);
    root = right;
    return treap(left);
  }

  /// other will be destroyed
  auto append(treap&& other) -> treap& {
    if (other.root != nullptr) {
      if (root == nullptr) root = other.root;
      else root = _join(root, other.root);
      other.root = nullptr;
    }
    return *this;
  }

  /// assumes add is not NULL
  auto _push_back(node_t* add) -> node_t* {
    if (root == nullptr) root = add;
    else root = _join(root, add);
    return add;
  }

  template <search_params params, typename... Args>
  auto _insert(node_t* add, Args&&... args) -> node_t* {
    auto [left, right] = _split<params>(std::forward<Args>(args)...);
    root = _join_with_mid(left, add, right);
    return add;
  }

  /// only supports by key
  template <typename Key_t, typename... ValueArgs>
  auto _try_emplace(Key_t const& key, ValueArgs&&... args) -> node_t* {
    node_t* left = nullptr;
    node_t* right = nullptr;
    node_t* found = _try_split(root, left, right, key);
    if (found != nullptr) return found;
    set_parent(left, nullptr);
    set_parent(right, nullptr);
    node_t* add = new_node(key, std::forward<ValueArgs>(args)...);
    root = _join_with_mid(left, add, right);
    return add;
  }

  /// cur can be NULL. returns ptr to node if it already exists
  /// only sets left/right if ptr returned is nullptr
  /// always by key
  template <typename Key_t>
  auto _try_split(node_t* cur, node_t*& left, node_t*& right, Key_t const& key) -> node_t* {
    if (cur == nullptr) {
      left = right = nullptr;
      return nullptr;
    }
    push(cur);
    if (key < cur->key) {
      node_t* found = _try_split(cur->left, left, cur->left, key);
      if (found == nullptr) {
        set_parent(cur->left, cur);
        right = cur;
        pull(cur);
      }
      return found;
    } else if (cur->key < key) {
      node_t* found = _try_split(cur->right, cur->right, right, key);
      if (found == nullptr) {
        set_parent(cur->right, cur);
        left = cur;
        pull(cur);
      }
      return found;
    } else {
      return cur;
    }
  }

  /// lower specifies lower_bound or upper_bound
  template <search_params params, typename... Args>
  auto _split(Args&&... args) -> std::pair<node_t*, node_t*> {
    node_t* left = nullptr;
    node_t* right = nullptr;
    _split_rec<params>(root, left, right, std::forward<Args>(args)...);
    set_parent(left, nullptr);
    set_parent(right, nullptr);
    return std::pair(left, right);
  }

  /// cur can be NULL
  template <search_params params, typename... Args>
    requires(not params.has_all(params.BY_KEY) or sizeof...(Args) == 1)
  auto _split_rec(node_t* cur, node_t*& left, node_t*& right, Args&&... args) -> void {
    if (cur == nullptr) {
      left = right = nullptr;
      return;
    }
    push(cur);
    bool const go_left = [&] {
      if constexpr (params.has_all(params.BY_KEY)) {
        if constexpr (params.has_all(params.UPPER_BOUND)) {
          return (args, ...) < cur->key;
        } else {
          return (args, ...) <= cur->key;
        }
      } else {
        if constexpr (params.has_all(params.UPPER_BOUND)) {
          return cur->search(args...) < 0;
        } else {
          return cur->search(args...) <= 0;
        }
      }
    }();
    if (go_left) {
      _split_rec<params>(cur->left, left, cur->left, std::forward<Args>(args)...);
      set_parent(cur->left, cur);
      right = cur;
    } else {
      _split_rec<params>(cur->right, cur->right, right, std::forward<Args>(args)...);
      set_parent(cur->right, cur);
      left = cur;
    }
    pull(cur);
  }

  /// assumes mid is not NULL
  /// left and right can be NULL
  auto _join_with_mid(node_t* left, node_t* mid, node_t* right) -> node_t* {
    if (left != nullptr) mid = _join(left, mid);
    if (right != nullptr) mid = _join(mid, right);
    return mid;
  }

  /// assumes left and right are not null
  auto _join(node_t* left, node_t* right) -> node_t* {
    if (left->heap_depth < right->heap_depth) {
      push(left);
      left->right = left->right == nullptr ? right : _join(left->right, right);
      set_parent(left->right, left);
      pull(left);
      return left;
    } else {
      push(right);
      right->left = right->left == nullptr ? left : _join(left, right->left);
      set_parent(right->left, right);
      pull(right);
      return right;
    }
  }
};

template <
    template <typename> typename AllocT, typename key_t, typename value_t,
    treap_traits traits = treap_traits::NONE>
using make_treap_map_allocator = AllocT<treap_map_node<key_t, value_t, traits>>;

template <
    typename key_t, typename value_t, treap_traits traits = treap_traits::NONE,
    typename Alloc = std::allocator<treap_map_node<key_t, value_t, traits>>>
using treap_map = treap<treap_map_node<key_t, value_t, traits>, Alloc>;
