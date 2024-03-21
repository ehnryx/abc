/* Segment Tree
 * USAGE
 *  segment_tree<node_t> segtree(n); initializes a segment tree with >= n leaves
 *  segment_tree<node_t> segtree(begin, end); initializes a segment tree with given values
 *  node_t is a class to be provided:
 *    STANDARD
 *      void put(args...); update at node
 *      out_t get(args...); gets the return value from node
 *      static out_t merge(out_t const& l, out_t const& r, args...); merges the return values
 *      void pull(node_t const& l, node_t const& r); pulls values from children
 *      void push(node_t& l, node_t& r); pushes lazy to l and r
 *      accumulate_t<type> accumulate(accumulate_t<type>, args...); for no-push lazy
 *        (accumulating the lazy values on the path from root to node on query)
 *    BINARY SEARCH
 *      bool contains(args&...); does the segment contain the value? update args if not
 *    BEATS
 *      bool update_break_cond(args...); whether to break in segtree beats
 *      bool update_put_cond(args...); whether to update in segtree beats
 *      conditions only apply to range updates
 *    PERSISTENT
 *      bool should_push(); to save some memory
 * MEMBERS
 *  update_range(l, r, value...); value for range update
 *  query_range(l, r, ...); query with optional args
 *  update_point(x, value...); point update
 *  query_point(x, ...); point query
 *  search_left(l, r, ...); search on the segtree, starting from the left
 *  search_right(l, r, ...); search on the segtree, starting from the right
 *  returns segment_tree.lim if not found (ie. n in the constructor)
 *  All ranges are inclusive
 * TIME
 *  O(logN) per query
 *  N = |array|
 * STATUS [normal]
 *  tested: cf/380c,609f,501d,19d,220e,474f,339d,292e
 * STATUS [sparse]
 *  tested: cf/501d,474f,609f,292e; kattis/precariousstacks
 * STATUS [persistent]
 *  tested: cf/501d,474f,292e,707d,786c
 */
#pragma once

#include "utility/traits.h"

#include <bit>
#include <stdexcept>
#include <vector>

// clang-format off
MAKE_TRAITS(segment_tree_traits,
  (SPARSE, PERSISTENT, NO_CHECKS),
);
// clang-format on

struct segment_length_t {
  size_t value;
  explicit segment_length_t(size_t v) : value(v) {}
  explicit segment_length_t(int v) : value(v) {}
  explicit segment_length_t(int64_t v) : value(v) {}
  explicit operator size_t() const { return value; }
  explicit operator int() const { return (int)value; }
  explicit operator int64_t() const { return value; }
  segment_length_t operator/(size_t c) { return segment_length_t{value / c}; }
  segment_length_t operator*(size_t c) { return segment_length_t{value / c}; }
};

template <typename node_t, typename acc_t, typename... Args>
concept has_segment_accumulate_without_length =
    requires(node_t nd, acc_t const& acc, Args const&... args) { nd.accumulate(acc, args...); };
template <typename node_t, typename acc_t, typename... Args>
concept has_segment_accumulate_with_length =
    requires(node_t nd, acc_t const& acc, segment_length_t sl, Args const&... args) {
      nd.accumulate(acc, sl, args...);
    };
template <typename node_t, typename acc_t, typename... Args>
concept has_segment_accumulate = has_segment_accumulate_with_length<node_t, acc_t, Args...> or
                                 has_segment_accumulate_without_length<node_t, acc_t, Args...>;

struct segment_tree_children_t {
  int left = 0;
  int right = 0;
};

template <typename node_t, segment_tree_traits traits>
struct segment_tree_data {
  int lim, length;
  std::vector<node_t> data;
  auto operator[](int i) -> node_t& { return data[i]; }
  auto root() -> node_t& { return data[1]; }

  static auto get_power2(int n) -> int {
    return 1 << (n <= 1 ? 0 : 32 - std::countl_zero((unsigned)n - 1));
  }
  segment_tree_data(int n) : lim(n), length(get_power2(lim)), data(2 * length) {}
  segment_tree_data(int n, node_t init)
      : lim(n), length(get_power2(lim)), data(2 * length, init) {}
  template <std::input_iterator input_it>
  segment_tree_data(input_it s, input_it t)
      : lim((int)std::distance(s, t)), length(get_power2(lim)), data(2 * length) {
    std::copy(s, t, data.begin() + length);
    build();
  }
  template <typename container>
    requires(requires(container c) {
      { std::begin(c) } -> std::input_iterator;
      { std::end(c) } -> std::input_iterator;
    })
  segment_tree_data(container const& c) : segment_tree_data(std::begin(c), std::end(c)) {}
  auto build() -> void {
    if constexpr (requires(node_t a, node_t b, node_t c) { a.pull(b, c); }) {
      for (int i = length - 1; i > 0; i--) {
        data[i].pull(data[get_left(i)], data[get_right(i)]);
      }
    }
  }
  static auto get_left(int i) -> int { return 2 * i; }
  static auto get_right(int i) -> int { return 2 * i + 1; }
};

template <typename node_t, segment_tree_traits traits>
  requires(bool((traits & traits.SPARSE)))
struct segment_tree_data<node_t, traits> {
  int64_t lim, length;
  std::vector<node_t> data;
  std::vector<segment_tree_children_t> children;

  auto root() -> node_t& { return data[1]; }

  static auto get_power2(int64_t n) -> int64_t {
    return 1ll << (n <= 1 ? 0 : 64 - std::countl_zero((uint64_t)n - 1));
  }
  segment_tree_data(int64_t n, size_t capacity = 0)
      : lim(n), length(get_power2(lim)), data(2), children(2) {
    if (capacity) {
      data.reserve(capacity);
      children.reserve(capacity);
    }
  }
  auto get_left(int i) const -> int { return children[i].left; }
  auto get_right(int i) const -> int { return children[i].right; }
  auto make_left(int i) -> void {
    if (not get_left(i)) {
      children[i].left = (int)data.size();
      data.emplace_back();
      children.emplace_back();
    }
  }
  auto make_right(int i) -> void {
    if (not get_right(i)) {
      children[i].right = (int)data.size();
      data.emplace_back();
      children.emplace_back();
    }
  }
};

template <typename node_t>
struct persistent_segment_tree_data {
  std::vector<char> pushed;
  static constexpr bool has_should_push = false;
};

template <typename node_t>
  requires(requires(node_t node_v) { node_v.should_push(); })
struct persistent_segment_tree_data<node_t> {
  static constexpr bool has_should_push = true;
};

template <typename node_t, segment_tree_traits traits>
  requires(bool((traits & traits.PERSISTENT)))
struct segment_tree_data<node_t, traits> : persistent_segment_tree_data<node_t> {
  using persistent_segment_tree_data<node_t>::has_should_push;
  int64_t lim, length;
  std::vector<int> version_roots;
  std::vector<node_t> data;
  std::vector<segment_tree_children_t> children;

  auto root(int version) -> node_t& { return data[get_root(version)]; }

  static auto get_power2(int64_t n) -> int64_t {
    return 1ll << (n <= 1 ? 0 : 64 - std::countl_zero((uint64_t)n - 1));
  }
  segment_tree_data(int64_t n, size_t capacity = 0)
      : persistent_segment_tree_data<node_t>(), lim(n), length(get_power2(lim)),
        version_roots(1) {
    if (capacity) {
      data.reserve(capacity);
      children.reserve(capacity);
    }
    data.resize(2);
    children.resize(2);
    if constexpr (not has_should_push) {
      this->pushed.reserve(capacity);
      this->pushed.resize(2);
    }
  }
  auto get_root(int version) const -> int { return version_roots[version]; }
  auto get_left(int i) const -> int { return children[i].left; }
  auto get_right(int i) const -> int { return children[i].right; }
  template <typename... Args>
  auto make_node(int i) -> int {
    data.emplace_back(data[i]);
    children.emplace_back(children[i]);
    if constexpr (not has_should_push) {
      this->pushed.emplace_back(false);
    }
    return (int)data.size() - 1;
  }
  auto skip_push(int i) -> bool {
    if constexpr (has_should_push) {
      return not data[i].should_push();
    } else {
      if (this->pushed[i]) {
        return true;
      } else {
        this->pushed[i] = true;
        return false;
      }
    }
  }
};

template <typename Node_t, segment_tree_traits traits = segment_tree_traits::NONE>
  requires(traits.count(traits.SPARSE | traits.PERSISTENT) <= 1)
struct segment_tree : segment_tree_data<Node_t, traits> {
  static constexpr bool sparse = bool(traits & traits.SPARSE);
  static constexpr bool persistent = bool(traits & traits.PERSISTENT);
  static constexpr bool normal = not sparse and not persistent;
  static constexpr bool check_bounds = not(traits & traits.NO_CHECKS);
  using coordinate_t = std::conditional_t<normal, int, int64_t>;

  using segment_tree_data<Node_t, traits>::data;
  using segment_tree_data<Node_t, traits>::get_left;
  using segment_tree_data<Node_t, traits>::get_right;
  using segment_tree_data<Node_t, traits>::length;
  using segment_tree_data<Node_t, traits>::lim;

  using node_t = Node_t;

  template <typename... Args>
  using return_t = decltype(std::declval<node_t>().get(std::declval<Args>()...));

  using update_return_t = std::conditional_t<persistent, int, void>;

  static constexpr bool has_push_no_length =
      requires(node_t nd, node_t& nd_ref) { nd.push(nd_ref, nd_ref); };
  static constexpr bool has_push_with_length =
      requires(node_t nd, node_t& nd_ref, segment_length_t sl) { nd.push(nd_ref, nd_ref, sl); };
  static_assert(has_push_no_length + has_push_with_length <= 1);

  static constexpr bool has_pull =
      requires(node_t nd, node_t const& nd_cref) { nd.pull(nd_cref, nd_cref); };
  template <typename... Args>
  static constexpr bool use_pull_as_merge =
      has_pull && not requires(return_t<Args...> ret, Args const&... args) {
        node_t::merge(ret, ret, args...);
      } && std::is_same_v<node_t, return_t<Args...>>;

  template <typename... Args>
  segment_tree(Args&&... args)
      : segment_tree_data<Node_t, traits>(std::forward<Args>(args)...) {}

  // Updates

  template <typename... Args>
  auto push(int i, Args const&... args) -> void {
    if constexpr (sparse) {
      this->make_left(i);
      this->make_right(i);
    }
    if constexpr (persistent) {
      if (this->skip_push(i)) return;
      this->children[i].left = this->make_node(get_left(i));
      this->children[i].right = this->make_node(get_right(i));
    }
    data[i].push(data[get_left(i)], data[get_right(i)], args...);
  }

  template <typename... Args>
    requires(not requires(node_t nd, segment_length_t sl, Args const&... args) {
      nd.put(sl, args...);
    })
  auto put(int i, Args const&... args) -> update_return_t {
    data[i].put(args...);
    if constexpr (persistent) return i;
  }

  template <typename acc_t, typename... Args>
    requires(not has_segment_accumulate_with_length<node_t, acc_t, Args...>)
  auto accumulate(int i, acc_t acc, Args const&... args) -> acc_t {
    return data[i].accumulate(acc, args...);
  }

  template <typename... Args>
  auto update_range(coordinate_t l, coordinate_t r, Args const&... args) -> update_return_t
    requires(not persistent)
  {
    if constexpr (check_bounds) {
      if (r < l) return;  // empty range
      if (l < 0 || lim <= r) throw std::invalid_argument("update range out of bounds");
    }
    return _update_range(l, r, 1, 0, length - 1, args...);
  }
  template <typename... Args>
  auto update_range(int version, coordinate_t l, coordinate_t r, Args const&... args)
      -> update_return_t
    requires(persistent)
  {
    if constexpr (check_bounds) {
      if (r < l) return version;  // empty range
      if (l < 0 || lim <= r) throw std::invalid_argument("update range out of bounds");
      if (version >= (int)this->version_roots.size()) {
        throw std::invalid_argument("version does not exist");
      }
    }
    this->version_roots.push_back(
        _update_range(l, r, this->get_root(version), 0, length - 1, args...));
    return (int)this->version_roots.size() - 1;
  }
  template <typename... Args>
    requires(
        not(persistent and
            (
                requires(node_t nd, Args const&... args) { nd.update_break_cond(args...); } or
                requires(node_t nd, Args const&... args) { nd.update_put_cond(args...); })))
  auto _update_range(
      coordinate_t const l, coordinate_t const r, int i, coordinate_t const seg_l,
      coordinate_t const seg_r, Args const&... args) -> update_return_t {
    if constexpr (requires { data[i].update_break_cond(args...); }) {
      if (data[i].update_break_cond(args...)) return;
    }
    if constexpr (not requires { data[i].update_put_cond(args...); }) {
      if (l <= seg_l && seg_r <= r) {
        if constexpr (persistent) i = this->make_node(i);
        if constexpr (requires { data[i].put(args...); }) return put(i, args...);
        else return put(i, segment_length_t{seg_r - seg_l + 1}, args...);
      }
    } else {  // can't be persistent
      if (l <= seg_l && seg_r <= r && data[i].update_put_cond(args...)) {
        if constexpr (requires { data[i].put(args...); }) return put(i, args...);
        else return put(i, segment_length_t{seg_r - seg_l + 1}, args...);
      }
      if constexpr (check_bounds) {
        if (i >= length) {
          throw std::invalid_argument(
              "update_put_cond/update_break_cond is incorrect, "
              "trying to descend past a leaf");
        }
      }
    }
    if constexpr (has_push_with_length) push(i, segment_length_t{seg_r - seg_l + 1});
    if constexpr (has_push_no_length) push(i);
    int const old_i = i;
    if constexpr (persistent) i = this->make_node(i);
    coordinate_t const mid = (seg_l + seg_r) / 2;
    if (l <= mid) {
      if constexpr (persistent) {
        this->children[i].left = _update_range(l, r, get_left(old_i), seg_l, mid, args...);
      } else {
        if constexpr (sparse) this->make_left(i);
        _update_range(l, r, get_left(i), seg_l, mid, args...);
      }
    }
    if (mid < r) {
      if constexpr (persistent) {
        this->children[i].right =
            _update_range(l, r, get_right(old_i), mid + 1, seg_r, args...);
      } else {
        if constexpr (sparse) this->make_right(i);
        _update_range(l, r, get_right(i), mid + 1, seg_r, args...);
      }
    }
    if constexpr (has_pull) data[i].pull(data[get_left(i)], data[get_right(i)]);
    if constexpr (persistent) return i;
  }

  template <typename... Args>
  auto update_point(coordinate_t x, Args const&... args) -> update_return_t
    requires(not persistent)
  {
    if constexpr (check_bounds) {
      if (x < 0 || lim <= x) throw std::invalid_argument("update_point index out of bounds");
    }
    return _update_point(x, 1, 0, length - 1, args...);
  }
  template <typename... Args>
  auto update_point(int version, coordinate_t x, Args const&... args) -> update_return_t
    requires(persistent)
  {
    if constexpr (check_bounds) {
      if (x < 0 || lim <= x) throw std::invalid_argument("update_point index out of bounds");
      if (version >= (int)this->version_roots.size()) {
        throw std::invalid_argument("version does not exist");
      }
    }
    this->version_roots.push_back(
        _update_point(x, this->get_root(version), 0, length - 1, args...));
    return (int)this->version_roots.size() - 1;
  }
  template <typename... Args>
  auto _update_point(
      coordinate_t const x, int i, coordinate_t const seg_l, coordinate_t const seg_r,
      Args const&... args) -> update_return_t {
    if (seg_l == seg_r) {
      if constexpr (persistent) i = this->make_node(i);
      if constexpr (requires { data[i].put(args...); }) return put(i, args...);
      else return put(i, segment_length_t{seg_r - seg_l + 1}, args...);
    }
    if constexpr (has_push_with_length) push(i, segment_length_t{seg_r - seg_l + 1});
    if constexpr (has_push_no_length) push(i);
    int const old_i = i;
    if constexpr (persistent) i = this->make_node(i);
    coordinate_t const mid = (seg_l + seg_r) / 2;
    if (x <= mid) {
      if constexpr (persistent) {
        this->children[i].left = _update_point(x, get_left(old_i), seg_l, mid, args...);
      } else {
        if constexpr (sparse) this->make_left(i);
        _update_point(x, get_left(i), seg_l, mid, args...);
      }
    } else {
      if constexpr (persistent) {
        this->children[i].right = _update_point(x, get_right(old_i), mid + 1, seg_r, args...);
      } else {
        if constexpr (sparse) this->make_right(i);
        _update_point(x, get_right(i), mid + 1, seg_r, args...);
      }
    }
    if constexpr (has_pull) data[i].pull(data[get_left(i)], data[get_right(i)]);
    if constexpr (persistent) return i;
  }

  // Queries

  template <typename... Args>
  auto query_range(coordinate_t l, coordinate_t r, Args const&... args) -> return_t<Args...>
    requires(requires(node_t nd) { nd.get(args...); } and not persistent)
  {
    if constexpr (check_bounds) {
      if (r < l) return data[0].get(args...);
      if (l < 0 || lim <= r) throw std::invalid_argument("query range out of bounds");
    }
    return _query_range(l, r, 1, 0, length - 1, args...);
  }
  template <typename... Args>
  auto query_range(int version, coordinate_t l, coordinate_t r, Args const&... args)
      -> return_t<Args...>
    requires(requires(node_t nd) { nd.get(args...); } and persistent)
  {
    if constexpr (check_bounds) {
      if (r < l) return data[0].get(args...);
      if (l < 0 || lim <= r) throw std::invalid_argument("query range out of bounds");
      if (version >= (int)this->version_roots.size()) {
        throw std::invalid_argument("version does not exist");
      }
    }
    return _query_range(l, r, this->get_root(version), 0, length - 1, args...);
  }

  template <typename acc_t, typename... Args>
    requires(has_segment_accumulate<node_t, acc_t, Args...>)
  auto _query_range(
      coordinate_t const l, coordinate_t const r, int const i, coordinate_t const seg_l,
      coordinate_t const seg_r, acc_t const& acc, Args const&... args)
      -> return_t<acc_t, Args...> {
    if constexpr (has_segment_accumulate_without_length<node_t, acc_t, Args...>) {
      return _query_range_impl(l, r, i, seg_l, seg_r, accumulate(i, acc, args...), args...);
    } else {
      return _query_range_impl(
          l, r, i, seg_l, seg_r, accumulate(i, acc, segment_length_t{seg_r - seg_l}, args...),
          args...);
    }
  }
  template <typename... Args>
  auto _query_range(
      coordinate_t const l, coordinate_t const r, int const i, coordinate_t const seg_l,
      coordinate_t const seg_r, Args const&... args) -> return_t<Args...> {
    return _query_range_impl(l, r, i, seg_l, seg_r, args...);
  }
  template <typename... Args>
  auto _query_range_impl(
      coordinate_t const l, coordinate_t const r, int const i, coordinate_t const seg_l,
      coordinate_t const seg_r, Args const&... args) -> return_t<Args...> {
    if (l <= seg_l && seg_r <= r) return data[i].get(args...);
    if constexpr (has_push_with_length) push(i, segment_length_t{seg_r - seg_l + 1});
    if constexpr (has_push_no_length) push(i);
    coordinate_t const mid = (seg_l + seg_r) / 2;
    bool go_left = (l <= mid);
    bool go_right = (mid < r);
    if constexpr (sparse or persistent) {
      go_left &= (get_left(i) != 0);
      go_right &= (get_right(i) != 0);
      if (not go_left and not go_right) {
        return data[0].get(args...);  // this can't happen right?
      }
    }
    if (not go_right) return _query_range(l, r, get_left(i), seg_l, mid, args...);
    if (not go_left) return _query_range(l, r, get_right(i), mid + 1, seg_r, args...);
    if constexpr (use_pull_as_merge<Args...>) {
      return node_t().pull(
          _query_range(l, r, get_left(i), seg_l, mid, args...),
          _query_range(l, r, get_right(i), mid + 1, seg_r, args...));
    } else {
      return node_t::merge(
          _query_range(l, r, get_left(i), seg_l, mid, args...),
          _query_range(l, r, get_right(i), mid + 1, seg_r, args...), args...);
    }
  }

  template <typename... Args>
  auto query_point(coordinate_t x, Args const&... args) -> return_t<Args...>
    requires(requires(node_t nd) { nd.get(args...); } and not persistent)
  {
    if constexpr (check_bounds) {
      if (x < 0 || lim <= x) throw std::invalid_argument("query_point index out of bounds");
    }
    return _query_point(x, 1, 0, length - 1, args...);
  }
  template <typename... Args>
  auto query_point(int version, coordinate_t x, Args const&... args) -> return_t<Args...>
    requires(requires(node_t nd) { nd.get(args...); } and persistent)
  {
    if constexpr (check_bounds) {
      if (x < 0 || lim <= x) throw std::invalid_argument("query_point index out of bounds");
      if (version >= (int)this->version_roots.size()) {
        throw std::invalid_argument("version does not exist");
      }
    }
    return _query_point(x, this->get_root(version), 0, length - 1, args...);
  }

  template <typename acc_t, typename... Args>
    requires(has_segment_accumulate<node_t, acc_t, Args...>)
  auto _query_point(
      coordinate_t const x, int const i, coordinate_t const seg_l, coordinate_t const seg_r,
      acc_t const& acc, Args const&... args) -> return_t<acc_t, Args...> {
    if constexpr (has_segment_accumulate_without_length<node_t, acc_t, Args...>) {
      return _query_point_impl(x, i, seg_l, seg_r, accumulate(i, acc, args...), args...);
    } else {
      return _query_point_impl(
          x, i, seg_l, seg_r, accumulate(i, acc, segment_length_t{seg_r - seg_l}, args...),
          args...);
    }
  }
  template <typename... Args>
  auto _query_point(
      coordinate_t const x, int const i, coordinate_t const seg_l, coordinate_t const seg_r,
      Args const&... args) -> return_t<Args...> {
    return _query_point_impl(x, i, seg_l, seg_r, args...);
  }
  template <typename... Args>
  auto _query_point_impl(
      coordinate_t const x, int const i, coordinate_t const seg_l, coordinate_t const seg_r,
      Args const&... args) -> return_t<Args...> {
    if (seg_l == seg_r) return data[i].get(args...);
    if constexpr (has_push_with_length) push(i, segment_length_t{seg_r - seg_l + 1});
    if constexpr (has_push_no_length) push(i);
    coordinate_t const mid = (seg_l + seg_r) / 2;
    if (x <= mid) {
      if constexpr (sparse or persistent) {
        if (not get_left(i)) return data[0].get(args...);
      }
      return _query_point(x, get_left(i), seg_l, mid, args...);
    } else {
      if constexpr (sparse or persistent) {
        if (not get_right(i)) return data[0].get(args...);
      }
      return _query_point(x, get_right(i), mid + 1, seg_r, args...);
    }
  }

  // Binary search

  template <typename... Args>
  auto search_left(coordinate_t l, coordinate_t r, Args... args) -> coordinate_t
    requires(not persistent)
  {
    if constexpr (check_bounds) {
      if (r < l) return lim;
      if (l < 0 || lim <= r) throw std::invalid_argument("search_left out of bounds");
    }
    return _search_left(l, r, 1, 0, length - 1, args...);
  }
  template <typename... Args>
  auto search_left(int version, coordinate_t l, coordinate_t r, Args... args) -> coordinate_t
    requires(persistent)
  {
    if constexpr (check_bounds) {
      if (r < l) return lim;
      if (l < 0 || lim <= r) throw std::invalid_argument("search_left out of bounds");
      if (version >= (int)this->version_roots.size()) {
        throw std::invalid_argument("version does not exist");
      }
    }
    return _search_left(l, r, this->get_root(version), 0, length - 1, args...);
  }
  template <typename... Args>
  auto _search_left(
      coordinate_t const l, coordinate_t const r, int const i, coordinate_t const seg_l,
      coordinate_t const seg_r, Args&... args) -> coordinate_t {
    if (l <= seg_l && seg_r <= r && not data[i].contains(args...)) return lim;
    if (seg_l == seg_r) return seg_l;
    if constexpr (has_push_with_length) push(i, segment_length_t{seg_r - seg_l + 1});
    if constexpr (has_push_no_length) push(i);
    coordinate_t const mid = (seg_l + seg_r) / 2;
    bool go_left = (l <= mid);
    bool go_right = (mid < r);
    if constexpr (sparse or persistent) {
      go_left &= (get_left(i) != 0);
      go_right &= (get_right(i) != 0);
    }
    coordinate_t res = (go_left ? _search_left(l, r, get_left(i), seg_l, mid, args...) : lim);
    if (res == lim && go_right) {
      res = _search_left(l, r, get_right(i), mid + 1, seg_r, args...);
    }
    return res;
  }

  template <typename... Args>
  auto search_right(coordinate_t l, coordinate_t r, Args... args) -> coordinate_t
    requires(normal or sparse)
  {
    if constexpr (check_bounds) {
      if (r < l) return lim;
      if (l < 0 || lim <= r) throw std::invalid_argument("search_right out of bounds");
    }
    return _search_right(l, r, 1, 0, length - 1, args...);
  }
  template <typename... Args>
  auto search_right(int version, coordinate_t l, coordinate_t r, Args... args) -> coordinate_t
    requires(persistent)
  {
    if constexpr (check_bounds) {
      if (r < l) return lim;
      if (l < 0 || lim <= r) throw std::invalid_argument("search_right out of bounds");
      if (version >= (int)this->version_roots.size()) {
        throw std::invalid_argument("version does not exist");
      }
    }
    return _search_right(l, r, this->get_root(version), 0, length - 1, args...);
  }
  template <typename... Args>
  auto _search_right(
      coordinate_t const l, coordinate_t const r, int const i, coordinate_t const seg_l,
      coordinate_t const seg_r, Args&... args) -> coordinate_t {
    if (l <= seg_l && seg_r <= r && not data[i].contains(args...)) return lim;
    if (seg_l == seg_r) return seg_l;
    if constexpr (has_push_with_length) push(i, segment_length_t{seg_r - seg_l + 1});
    if constexpr (has_push_no_length) push(i);
    coordinate_t const mid = (seg_l + seg_r) / 2;
    bool go_left = (l <= mid);
    bool go_right = (mid < r);
    if constexpr (sparse or persistent) {
      go_left &= (get_left(i) != 0);
      go_right &= (get_right(i) != 0);
    }
    coordinate_t res =
        (go_right ? _search_right(l, r, get_right(i), mid + 1, seg_r, args...) : lim);
    if (res == lim && go_left) {
      res = _search_right(l, r, get_left(i), seg_l, mid, args...);
    }
    return res;
  }
};
