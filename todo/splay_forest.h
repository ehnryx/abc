/* Splay Forest
 * USAGE
 *  gives a collection of nodes (not a single tree)
 *  pray
 * TIME
 *  O(logN) per operation amortized
 *  N = |splay tree|
 * STATUS
 *  untested: cf/104941f
 *  treap contest: 102787
 */
#pragma once

#include "todo/binary_search_traits.h"
#include "utility/traits.h"

#include <memory>

// clang-format off
MAKE_TRAITS(splay_traits,
  (ORDER_STATS),
);
// clang-format on

struct splay_index {};

struct splay_node_pointer {
  int32_t idx;
  operator int32_t() const { return idx; }
};

template <typename, splay_traits>
struct splay_node_data {
  splay_node_pointer parent;
  splay_node_pointer left = {0};
  splay_node_pointer right = {0};
};

template <typename derived_t, splay_traits traits>
  requires(traits.has_all(traits.ORDER_STATS))
struct splay_node_data<derived_t, traits> {
  splay_node_pointer parent;
  splay_node_pointer left = {0};
  splay_node_pointer right = {0};
  int32_t size = 1;
  auto _pull_splay_node_size(derived_t const* data) -> void {
    size = 1 + data[left].size + data[right].size;
  }
  template <std::integral index_t>
  auto search(derived_t const* data, index_t index, splay_index) const -> int_fast32_t {
    if (index < data[left].size) return -1;
    else if (data[left].size < index) return 1;
    else return 0;
  }
  template <std::integral index_t>
  auto descend_right(derived_t const* data, index_t& index, splay_index) const -> void {
    index -= data[left].size + 1;
  }
};

template <typename derived_t, typename Key_t, splay_traits traits = splay_traits::NONE>
struct splay_node_base : splay_node_data<derived_t, traits> {
  using key_t = Key_t;
  key_t key;
  splay_node_base(key_t&& k) : key(std::move(k)) {}
};

template <typename derived_t, splay_traits traits>
struct splay_node_base<derived_t, void, traits> : splay_node_data<derived_t, traits> {
  using key_t = void;
  splay_node_base() = default;
};

template <typename Node_t, typename Alloc = std::allocator<Node_t>>
  requires(std::is_trivially_destructible_v<Node_t>)  // idk how to call destructor
struct splay_forest : Alloc {
  using node_t = Node_t;
  using pointer_t = splay_node_pointer;

  //auto debug() {
  //  cout << "\n------------------------\n";
  //  for (int I = _buffer_size; I >= 0; I--) {
  //    pointer_t i = {I};
  //    cout << i << " [" << get(i).value << "] : (" << get(i).parent << " " << get(i).left << " "
  //         << get(i).right << ") " << get(i).size << "\n";
  //  }
  //  cout << "------------------------\n" << endl;
  //}

  node_t* data;
  int32_t const _buffer_size;
  int32_t _next_data;
  splay_forest(int32_t n) : data(Alloc::allocate(n + 1)), _buffer_size(n), _next_data(n) {
    if constexpr (requires { node_t(); }) {
      std::construct_at(data);
    } else {
      data->parent = data->left = data->right = {0};
      if constexpr (requires { data->size; }) {
        data->size = 0;
      }
    }
  }
  ~splay_forest() { Alloc::deallocate(data, _buffer_size + 1); }

  auto get(pointer_t x) -> node_t& { return data[x]; }
  auto get(pointer_t x) const -> node_t const& { return data[x]; }

  template <typename... Args>
  auto new_node(pointer_t parent, Args&&... args) -> pointer_t {
    std::construct_at(data + _next_data, std::forward<Args>(args)...);
    data[_next_data].parent = parent;
    return {_next_data--};
  }

  /// assumes x is not NULL
  auto pull(pointer_t x) -> void {
    if constexpr (requires { data[x]._pull_splay_node_size(data); }) {
      data[x]._pull_splay_node_size(data);
    }
    if constexpr (requires { data[x].pull(data); }) {
      data[x].pull(data);
    }
  }

  /// assumes x is not NULL
  auto pull_to(pointer_t x, pointer_t nil) -> void {
    static constexpr bool has_pull_size = requires { data[x]._pull_splay_node_size(data); };
    static constexpr bool has_pull = requires { data[x].pull(data); };
    if constexpr (has_pull_size or has_pull) {
      for (; x != nil; x = data[x].parent) {
        if constexpr (has_pull_size) {
          data[x]._pull_splay_node_size(data);
        }
        if constexpr (has_pull) {
          data[x].pull(data);
        }
      }
    }
  }

  /// assumes x is not NULL
  auto push(pointer_t x) -> void {
    if constexpr (requires { data[x].push(data); }) {
      data[x].push(data);
    }
  }

  /// assumes x is not NULL
  auto erase(pointer_t rem) -> pointer_t {
    _splay(rem);
    auto before = data[rem].left;
    auto after = data[rem].right;
    // destroy rem ?
    if (before == 0) {
      if (after != 0) data[after].parent = 0;
      return after;
    } else {
      data[before].parent = 0;
      if (after != 0) {
        data[after].parent = 0;
        return append(before, after);
      } else {
        return before;
      }
    }
  }

  template <typename... Args>
  auto descend_left(pointer_t x, Args&&... args) -> void {
    if constexpr (requires { data[x].descend_left(data, args...); }) {
      data[x].descend_left(data, args...);
    }
  }

  template <typename... Args>
  auto descend_right(pointer_t x, Args&&... args) -> void {
    if constexpr (requires { data[x].descend_right(data, args...); }) {
      data[x].descend_right(data, args...);
    }
  }

  /// assumes x is a root, and not NULL
  template <typename... Args>
  auto try_emplace(pointer_t x, Args&&... args) -> pointer_t {
    auto const [found, root] =
        _search<search_params::FIND | search_params::BY_KEY | search_params::EMPLACE>(
            x, std::forward<Args>(args)...);
    return found;
  }

  auto _update_left(pointer_t p, pointer_t add) -> pointer_t {
    data[add].parent = p;
    if (p != 0) data[p].right = add;
    return add;
  }

  auto _update_right(pointer_t p, pointer_t add) -> pointer_t {
    data[add].parent = p;
    if (p != 0) data[p].left = add;
    return add;
  }

  /// assumes x is a root, and not NULL.
  /// returns (lower_bound, root)
  template <search_params params, typename... Args>
  auto _search(pointer_t x, Args&&... args) -> std::pair<pointer_t, pointer_t> {
    pointer_t left_root = {0};
    pointer_t right_root = {0};
    pointer_t left_parent = {0};
    pointer_t right_parent = {0};
    int_fast32_t parity = 0;
    int_fast32_t went_left = 0;
    pointer_t result = {0};
    for (bool done = false; not done;) {
      auto const search_dir = [&] {
        if constexpr (params.has_any(params.BY_KEY)) {
          return binary_search_details::search<params>(
              data[x], binary_search_details::get_first(args...));
        } else {
          return binary_search_details::search<params>(data[x], data, args...);
        }
      }();
      bool const go_left = [&] {
        if constexpr (params.has_any(params.FIND)) return search_dir < 0;
        else return search_dir;
      }();
      bool const go_right = [&] {
        if constexpr (params.has_any(params.FIND)) return search_dir > 0;
        else return not search_dir;
      }();
      if (go_left) {
        result = x;  // update result
        if (data[x].left == 0) {
          if constexpr (not params.has_any(params.EMPLACE)) {
            break;
          } else {
            data[x].left = result = new_node(x, std::forward<Args>(args)...);
            done = true;
          }
        } else {
          descend_left(x, args...);
        }
        parity ^= 1;
        went_left = went_left << 1 | 1;
        x = data[x].left;
      } else if (go_right) {
        if (data[x].right == 0) {
          if constexpr (not params.has_any(params.EMPLACE)) {
            break;
          } else {
            data[x].right = result = new_node(x, std::forward<Args>(args)...);
            done = true;
          }
        } else {
          descend_right(x, args...);
        }
        parity ^= 1;
        went_left = went_left << 1;
        x = data[x].right;
      } else {
        result = x;  // update result
        break;
      }
      // top down splay update
      if (parity == 0) {
        if ((went_left & 1) == went_left >> 1) {
          _rotate(data[x].parent, data[data[x].parent].parent, went_left);
          if (went_left) {
            right_parent = _update_right(right_parent, data[x].parent);
            if (right_root == 0) right_root = right_parent;
          } else {
            left_parent = _update_left(left_parent, data[x].parent);
            if (left_root == 0) left_root = left_parent;
          }
        } else {
          if (went_left & 1) {
            left_parent = _update_left(left_parent, data[data[x].parent].parent);
            right_parent = _update_right(right_parent, data[x].parent);
          } else {
            right_parent = _update_right(right_parent, data[data[x].parent].parent);
            left_parent = _update_left(left_parent, data[x].parent);
          }
          if (left_root == 0) left_root = left_parent;
          if (right_root == 0) right_root = right_parent;
        }
        went_left = 0;
      }
    }
    if (parity) {
      if (went_left) {
        right_parent = _update_right(right_parent, data[x].parent);
        if (right_root == 0) right_root = right_parent;
      } else {
        left_parent = _update_left(left_parent, data[x].parent);
        if (left_root == 0) left_root = left_parent;
      }
    }
    if (left_root != 0) {
      data[left_parent].right = data[x].left;
      if (data[x].left != 0) data[data[x].left].parent = left_parent;
      data[x].left = left_root;
      data[left_root].parent = x;
      pull_to(left_parent, x);
    }
    if (right_root != 0) {
      data[right_parent].left = data[x].right;
      if (data[x].right != 0) data[data[x].right].parent = right_parent;
      data[x].right = right_root;
      data[right_root].parent = x;
      pull_to(right_parent, x);
    }
    data[x].parent = {0};  // x is now root
    pull(x);
    return std::pair(result, x);
  }

  /// assumes x is a root, and not NULL
  template <typename... Args>
  auto split_before(pointer_t x, Args&&... args) -> std::pair<pointer_t, pointer_t> {
    auto const [after, root] =
        _search<search_params::LOWER_BOUND>(x, std::forward<Args>(args)...);
    if (after == 0) return std::pair(root, pointer_t{0});
    _splay(after);  // after is now root
    pointer_t before = data[after].left;
    data[after].left = {0};
    if (before != 0) data[before].parent = {0};
    pull(after);
    return std::pair(before, after);
  }

  /// assumes left and right are roots, and not NULL
  auto append(pointer_t before, pointer_t after) -> pointer_t {
    before = _splay(_rightmost(before));
    data[before].right = after;
    data[after].parent = before;
    pull(before);
    return before;
  }

  /// assumes x is a root, and not NULL. does not splay
  auto _rightmost(pointer_t x) -> pointer_t {
    while (data[x].right != 0) {
      x = data[x].right;
    }
    return x;
  }

  /// assumes x is not NULL. does not splay
  auto find_root(pointer_t x) -> pointer_t {
    while (data[x].parent != 0) {
      x = data[x].parent;
    }
    return x;
  }

  /// assumes x is not NULL
  inline auto _splay(pointer_t const x, pointer_t const to = {0}) -> pointer_t {
    pointer_t p = data[x].parent;
    bool x_left = data[p].left == x;
    while (p != to and data[p].parent != to) {
      pointer_t const pp = data[p].parent;
      bool const p_left = data[pp].left == p;
      pointer_t const next_p = data[pp].parent;
      if (x_left == p_left) {
        _rotate(p, pp, p_left);
        _rotate(x, p, x_left);
      } else {
        _rotate(x, p, x_left);
        _rotate(x, pp, p_left);
      }
      x_left = data[next_p].left == pp;
      p = next_p;
    }
    if (p != to) {
      _rotate(x, p, x_left);
      x_left = data[to].left == p;
    }
    data[x].parent = to;
    if (to != 0) {
      if (x_left) data[to].left = x;
      else data[to].right = x;
    }
    pull(x);
    return x;
  }

  /// assumes x and p are both not NULL (p should be the parent of x)
  /// is_left is true iff x is the left child of p
  /// _rotate does not connect x to its new parent (ie. the old parent of p)
  inline auto _rotate(pointer_t const x, pointer_t const p, bool const is_left) -> void {
    data[p].parent = x;
    if (is_left) {
      data[p].left = data[x].right;
      if (data[p].left != 0) data[data[p].left].parent = p;
      data[x].right = p;
    } else {
      data[p].right = data[x].left;
      if (data[p].right != 0) data[data[p].right].parent = p;
      data[x].left = p;
    }
    pull(p);
  }
};
