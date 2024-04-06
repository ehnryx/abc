/* Splay Forest
 * USAGE
 *  gives a collection of nodes (not a single tree)
 *  pray
 * TIME
 *  O(logN) per operation amortized
 *  N = |splay tree|
 * STATUS
 *  [pull/search]
 *    tested: boj/7480,16586,16994,17607
 *  [push]
 *    tested: kattis/precariousstacks; boj/3444,9990
 *  [map]
 *    tested: cf/104941f
 *  [TODO]
 *    untested: cf/102787
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
  uint32_t idx;
  operator uint32_t() const { return idx; }
};

namespace splay_details {
template <typename derived_t>
struct push_pull_dispatcher {
  static constexpr bool has_pull_user = requires(derived_t x) { x.pull(&x); };
  static constexpr bool has_pull_size = requires(derived_t x) { x._pull_splay_node_size(&x); };
  static constexpr bool has_pull = has_pull_user or has_pull_size;
  auto _pull_dispatcher(derived_t const* data) -> void {
    if constexpr (has_pull_user) {
      static_cast<derived_t*>(this)->pull(data);
    }
    if constexpr (has_pull_size) {
      static_cast<derived_t*>(this)->_pull_splay_node_size(data);
    }
  }
  static constexpr bool has_push_user = requires(derived_t x) { x.push(&x); };
  static constexpr bool has_push = has_push_user;
  auto _push_dispatcher(derived_t* data) -> void {
    if constexpr (has_push_user) {
      static_cast<derived_t*>(this)->push(data);
    }
  }
};

template <typename derived_t, splay_traits>
struct splay_node_data : push_pull_dispatcher<derived_t> {
  splay_node_pointer parent = {0};
  splay_node_pointer left = {0};
  splay_node_pointer right = {0};
};

template <typename derived_t, splay_traits traits>
  requires(traits.has_all(traits.ORDER_STATS))
struct splay_node_data<derived_t, traits> : push_pull_dispatcher<derived_t> {
  splay_node_pointer parent = {0};
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
}  // namespace splay_details

template <typename derived_t, typename Key_t, splay_traits traits = splay_traits::NONE>
struct splay_node_base : splay_details::splay_node_data<derived_t, traits> {
  using key_t = Key_t;
  key_t key;
  template <typename Key>
  splay_node_base(Key&& k) : key(std::move(k)) {}
};

template <typename derived_t, splay_traits traits>
struct splay_node_base<derived_t, void, traits>
    : splay_details::splay_node_data<derived_t, traits> {
  using key_t = void;
  splay_node_base() = default;
};

template <typename Node_t, typename Alloc = std::allocator<Node_t>>
  requires(std::is_trivially_destructible_v<Node_t>)  // idk how to call destructor
struct splay_forest : Alloc {
  using node_t = Node_t;
  using pointer_t = splay_node_pointer;

  node_t* data;
  uint32_t const _buffer_size;
  uint32_t _next_data;
  /// n should be less than std::numeric_limits<uint32_t>::max()
  splay_forest(uint32_t n) : data(Alloc::allocate(n + 1)), _buffer_size(n), _next_data(n) {
    if constexpr (requires { node_t(); }) {
      std::construct_at(data);
    } else {
      data->parent = data->left = data->right = {0};
    }
    if constexpr (requires { data->size; }) {
      data->size = 0;
    }
  }
  ~splay_forest() { Alloc::deallocate(data, _buffer_size + 1); }

  inline auto get(pointer_t x) -> node_t& { return data[x]; }
  inline auto get(pointer_t x) const -> node_t const& { return data[x]; }
  inline auto operator[](pointer_t x) -> node_t& { return get(x); }
  inline auto operator[](pointer_t x) const -> node_t const& { return get(x); }

  template <typename... Args>
  auto new_node(Args&&... args) -> pointer_t {
    std::construct_at(data + _next_data, std::forward<Args>(args)...);
    return pointer_t{_next_data--};
  }
  template <typename... Args>
  auto new_node_at(pointer_t x, Args&&... args) -> pointer_t {
    std::construct_at(data + x, std::forward<Args>(args)...);
    return x;
  }
  template <search_params params, typename... Args>
    requires(not params.has_any(params.INSERT))
  auto _get_node(Args&&... args) -> pointer_t {
    return new_node(std::forward<Args>(args)...);
  }
  template <search_params params>
    requires(params.has_any(params.INSERT))
  auto _get_node(pointer_t x, ...) -> pointer_t {
    return x;
  }

  /// assumes x is not NULL
  auto pull(pointer_t x) -> void {
    if constexpr (node_t::has_pull) {
      get(x)._pull_dispatcher(data);
    }
  }
  /// assumes x is not NULL
  auto pull_from(pointer_t x) -> void {
    if constexpr (node_t::has_pull) {
      for (; x != 0; x = get(x).parent) {
        pull(x);
      }
    }
  }
  /// assumes x is not NULL
  auto push(pointer_t x) -> void {
    if constexpr (node_t::has_push) {
      get(x)._push_dispatcher(data);
    }
  }
  /// assumes x is not NULL. x will get pushed
  auto push_to(pointer_t x) -> void {
    if constexpr (node_t::has_push) {
      if (get(x).parent != 0) push_to(get(x).parent);
      push(x);
    }
  }

  /// assumes x is not NULL
  auto erase(pointer_t rem) -> pointer_t {
    return _erase_root(splay(rem));  // push to + splay + erase root
  }
  /// assumes x is root, and not NULL
  template <search_params params, typename... Args>
  auto find_erase(pointer_t x, Args... args) -> pointer_t {
    auto [found, root] = _search<params>(x, args...);
    if (found == 0) return root;
    return _erase_root(found);
  }
  /// assumes x is root and not NULL
  auto _erase_root(pointer_t rem) -> pointer_t {
    auto before = get(rem).left;
    auto after = get(rem).right;
    // destroy rem ?
    if (before == 0) {
      if (after != 0) get(after).parent = {0};
      return after;
    } else {
      get(before).parent = {0};
      if (after != 0) {
        get(after).parent = {0};
        return _append(before, after);
      } else {
        return before;
      }
    }
  }

#define _SPLAY_UPDATE_LEFT(X) \
  do { \
    get(X).parent = left_parent; \
    if (left_root == 0) left_root = X; \
    else get(left_parent).right = X; \
    left_parent = X; \
  } while (false);

#define _SPLAY_UPDATE_RIGHT(X) \
  do { \
    get(X).parent = right_parent; \
    if (right_root == 0) right_root = X; \
    else get(right_parent).left = X; \
    right_parent = X; \
  } while (false)

  /// assumes x is root, and not NULL.
  /// returns (result, root)
  template <search_params params, typename... Args>
  auto _search(pointer_t x, Args... args) -> std::pair<pointer_t, pointer_t> {
    pointer_t left_root = {0};
    pointer_t right_root = {0};
    pointer_t left_parent = {0};
    pointer_t right_parent = {0};
    int_fast32_t parity = 0;
    int_fast32_t went_left = 0;
    bool done = false;
    while (not done) {
      push(x);  // push before descending
      auto const search_dir = [&] {
        if constexpr (params.has_any(params.BY_KEY) and not params.has_any(params.INSERT)) {
          return binary_search_details::search<params>(get(x), args...);
        } else {
          return binary_search_details::search<params>(get(x), data, args...);
        }
      }();
      bool const go_left = [&] {
        if constexpr (params.has_any(params.FIND)) return search_dir < 0;
        else return search_dir;
      }();
      bool const go_right = [&] {
        if constexpr (params.has_any(params.FIND)) return search_dir > 0;
        else return not go_left;
      }();
      if (go_left) {
        if (get(x).left == 0) {
          done = true;
          if constexpr (params.has_any(params.EMPLACE | params.INSERT)) {
            get(x).left = _get_node<params>(args...);
            get(get(x).left).parent = x;
          } else {
            break;
          }
        } else {
          if constexpr (not params.has_any(params.BY_KEY)) {
            binary_search_details::descend_left<params>(get(x), data, args...);
          }
        }
        parity ^= 1;
        went_left = went_left << 1 | 1;
        x = get(x).left;
      } else if (go_right) {
        if (get(x).right == 0) {
          done = true;
          if constexpr (params.has_any(params.EMPLACE | params.INSERT)) {
            get(x).right = _get_node<params>(args...);
            get(get(x).right).parent = x;
          } else {
            if constexpr (not params.has_any(params.FIND)) done = false;
            break;
          }
        } else {
          if constexpr (not params.has_any(params.BY_KEY)) {
            binary_search_details::descend_right<params>(get(x), data, args...);
          }
        }
        parity ^= 1;
        went_left = went_left << 1;
        x = get(x).right;
      } else {
        break;
      }
      // top down splay update
      if (parity == 0) {
        pointer_t const p = get(x).parent;
        pointer_t const pp = get(p).parent;
        if ((went_left & 1) == went_left >> 1) {
          _rotate(p, pp, went_left);
          if (went_left) _SPLAY_UPDATE_RIGHT(p);
          else _SPLAY_UPDATE_LEFT(p);
        } else {
          if (went_left & 1) {
            _SPLAY_UPDATE_LEFT(pp);
            _SPLAY_UPDATE_RIGHT(p);
          } else {
            _SPLAY_UPDATE_RIGHT(pp);
            _SPLAY_UPDATE_LEFT(p);
          }
        }
        went_left = 0;
      }
    }
    if (parity) {
      pointer_t const p = get(x).parent;
      if (went_left) _SPLAY_UPDATE_RIGHT(p);
      else _SPLAY_UPDATE_LEFT(p);
    }
    if (left_root != 0) {
      get(left_parent).right = get(x).left;
      if (get(left_parent).right != 0) get(get(left_parent).right).parent = left_parent;
      pull_from(left_parent);
      get(x).left = left_root;
      get(left_root).parent = x;
    }
    if (right_root != 0) {
      get(right_parent).left = get(x).right;
      if (get(right_parent).left != 0) get(get(right_parent).left).parent = right_parent;
      pull_from(right_parent);
      get(x).right = right_root;
      get(right_root).parent = x;
    }
    get(x).parent = {0};  // x is now root
    pull(x);
    auto const result = [&] {
      if constexpr (params.has_any(params.FIND) and not params.has_any(params.EMPLACE)) {
        if (done) return pointer_t{0};
      }
      if constexpr (params.has_any(params.FIND | params.EMPLACE | params.INSERT)) {
        return x;
      } else if constexpr (params.has_any(params.GET_LEFT)) {
        return done ? left_parent : x;
      } else {
        return done ? x : right_parent;
      }
    }();
    if constexpr (params.has_any(params.MAKE_ROOT)) {
      if (result and result != x) x = _splay(result);
    }
    return std::pair(result, x);
  }

#undef _SPLAY_UPDATE_LEFT
#undef _SPLAY_UPDATE_RIGHT

  /// assumes x is root or NULL
  /// returns [left_root, right_root]
  template <search_params params, typename... Args>
  auto split(pointer_t x, Args... args) -> std::pair<pointer_t, pointer_t> {
    if (x == 0) return std::pair(x, x);
    return _split<params>(x, args...);
  }
  /// assumes x is root, and not NULL
  /// returns [left_root, right_root]
  template <search_params params, typename... Args>
  auto _split(pointer_t x, Args... args) -> std::pair<pointer_t, pointer_t> {
    auto const [after, root] = _search<params>(x, args...);
    if (after == 0) return std::pair(root, pointer_t{0});
    _splay(after);  // after is now root. should be consistent with append
    return std::pair(_split_before_root(after), after);
  }
  /// assumes x is root, and not NULL
  /// returns left_root. x remains right_root
  auto _split_before_root(pointer_t after) -> pointer_t {
    pointer_t before = get(after).left;
    get(after).left = {0};
    if (before != 0) get(before).parent = {0};
    pull(after);
    return before;
  }

  /// assumes left and right are roots or NULL
  auto append(pointer_t before, pointer_t after) -> pointer_t {
    if (before == 0) return after;
    if (after == 0) return before;
    return _append(before, after);
  }
  /// assumes left and right are roots, and not NULL
  auto _append(pointer_t before, pointer_t after) -> pointer_t {
    after = _splay(_leftmost(after));  // after is root. should be consistent with split
    get(after).left = before;
    get(before).parent = after;
    pull(after);
    return after;
  }

  /// assumes x is not NULL. does not splay
  auto _rightmost(pointer_t x) -> pointer_t {
    while (get(x).right != 0) {
      push(x);
      x = get(x).right;
    }
    push(x);
    return x;
  }
  /// assumes x is not NULL. does not splay. x will get pushed
  auto _leftmost(pointer_t x) -> pointer_t {
    while (get(x).left != 0) {
      push(x);
      x = get(x).left;
    }
    push(x);
    return x;
  }

  /// does not splay. x will not get pushed. returns NULL if x is NULL
  auto find_root(pointer_t x) -> pointer_t {
    while (get(x).parent != 0) {
      x = get(x).parent;
    }
    return x;
  }

  /// assumes x is not NULL
  auto splay(pointer_t x) -> pointer_t {
    push_to(x);
    return _splay(x);
  }
  /// assumes x is not NULL, assumes no lazy on path from root to x (inclusive)
  inline auto _splay(pointer_t const x) -> pointer_t {
    pointer_t p = get(x).parent;
    bool x_left = get(p).left == x;
    while (p != 0 and get(p).parent != 0) {
      pointer_t const pp = get(p).parent;
      bool const p_left = get(pp).left == p;
      pointer_t const next_p = get(pp).parent;
      if (x_left == p_left) {
        _rotate(p, pp, p_left);
        _rotate(x, p, x_left);
      } else {
        _rotate(x, p, x_left);
        _rotate(x, pp, p_left);
      }
      x_left = get(next_p).left == pp;
      p = next_p;
    }
    if (p != 0) {
      _rotate(x, p, x_left);
    }
    get(x).parent = {0};
    pull(x);
    return x;
  }

  /// assumes x and p are both not NULL (p should be the parent of x)
  /// is_left is true iff x is the left child of p
  /// _rotate does not connect x to its new parent (ie. the old parent of p)
  inline auto _rotate(pointer_t const x, pointer_t const p, bool const is_left) -> void {
    get(p).parent = x;
    if (is_left) {
      get(p).left = get(x).right;
      if (get(p).left != 0) get(get(p).left).parent = p;
      get(x).right = p;
    } else {
      get(p).right = get(x).left;
      if (get(p).right != 0) get(get(p).right).parent = p;
      get(x).left = p;
    }
    pull(p);
  }

  template <typename Function>
  auto visit(pointer_t x, Function&& f) -> void {
    if (x == 0) return;
    push(x);
    visit(get(x).left, f);
    f(get(x));
    visit(get(x).right, std::move(f));
  };
};
