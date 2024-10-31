/* Fenwick Tree
 * USAGE
 *  fenwick_tree<T> range_sum(n); // 0-indexed
 *  range_sum.query_range(l, r);
 *  range_sum.query_point(x);
 *  range_sum.update_range(l, r, v);
 *  range_sum.update_point(x, v);
 *  range_sum.lower_bound(v);
 *    returns smallest r such that v <= sum_{0 <= i <= r} v_i
 *    returns n if no such r exists
 * TIME
 *  O(logN) per query, update, lower_bound
 * STATUS
 *  untested: cf/1515i,104772h
 */
#pragma once

#include <bit>
#include <stdexcept>
#include <vector>

template <typename T>
struct fenwick_tree {
  int const n, logn;
  std::vector<T> data;
  fenwick_tree(int _n) : n(_n), logn(std::bit_width(unsigned(n)) - 1), data(n + 1) {}
  template <std::input_iterator input_it>
  fenwick_tree(input_it s, input_it t) : fenwick_tree(int(std::distance(s, t))) {
    for (int i = 0; s != t; s++, i++) {
      update(i, *s);
    }
  }
  auto query_point(int r) const -> T { return query_range(r, r); }
  auto query_range(int l, int r) const -> T { return _query(r) - _query(l - 1); }
  auto _query(int r) const -> T {
    if (r <= -1) return 0;
    if (r >= n) throw std::invalid_argument("query index out of bounds");
    T res = 0;
    for (r += 1; r > 0; r -= r & -r) {
      res += data[r];
    }
    return res;
  }
  auto update(int i, T const& v) -> void {
    if (i < 0 || n <= i) throw std::invalid_argument("update index out of bounds");
    for (i += 1; i <= n; i += i & -i) {
      data[i] += v;
    }
  }
  auto lower_bound(T const& v) const -> int {
    int res = 0;
    T prefix = 0;
    for (int i = logn; i >= 0; i--) {
      if (res + (1 << i) <= n && prefix + data[res + (1 << i)] < v) {
        res += 1 << i;
        prefix += data[res];
      }
    }
    return res;
  }
};
