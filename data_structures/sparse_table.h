/* Sparse Table
 * USAGE
 *  sparse_table<Type, Functional> rq(arr);
 *  auto val = rq.query(l, r);
 *    inclusive range [l, r]
 *    assumes l <= r
 * CONSTRUCTOR
 *  arr: array on which to build the sparse table
 * TIME
 *  O(NlogN) construction, O(Functional) query
 *  N = |array|
 * STATUS
 *  untested: cf/1454f
 */
#pragma once

#include <bit>
#include <vector>

template <typename T, typename Func>
struct sparse_table {
  size_t const n;
  std::vector<T> data;
  template <std::input_iterator input_it>
  sparse_table(input_it s, input_it t) : n(std::distance(s, t)), data(n * std::bit_width(n)) {
    std::copy(s, t, data.begin());
    for (size_t j = 1; j < std::bit_width(n); j++) {
      for (size_t i = 0; i + (1 << j) <= n; i++) {
        data[j * n + i] = Func()(data[(j - 1) * n + i], data[(j - 1) * n + i + (1 << (j - 1))]);
      }
    }
  }
  auto query(size_t l, size_t r) const -> T {
    auto const layer = std::bit_width(r + 1 - l) - 1;
    return Func()(data[layer * n + l], data[layer * n + r + 1 - (1 << layer)]);
  }
};
