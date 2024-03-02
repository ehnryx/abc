/** N-Dimensional Array
 * USAGE
 *  nd_array<int, 3> arr(n1, n2, n3, initial_value);
 *  nd_array<int, 3> arr(n1, n2, n3);
 *  arr(i, j, k) for access (instead of arr[i][j][k])
 * NOTES
 *  stored in row-major order in a flattened vector
 *  unable to resize
 * STATUS
 *  tested: cf/46e
 */
#pragma once

#include "utility/nd_indexer.h"

#include <vector>

template <typename T, size_t ndims>
struct nd_array {
  nd_indexer<ndims> const indexer;
  std::vector<T> data;
  template <typename... Args>
    requires(sizeof...(Args) == ndims)
  nd_array(Args... ds) : indexer(ds...), data(indexer.size()) {}
  template <typename... Args>
    requires(sizeof...(Args) == ndims + 1)
  nd_array(Args... ds)
      : indexer(ds...), data(indexer.size(), ([](auto x) { return x; }(ds), ...)) {}
  auto size() const -> size_t { return indexer.size(); }
  auto operator[](size_t i) -> T& { return data[i]; }
  auto operator[](size_t i) const -> T const& { return data[i]; }
  auto from_index(size_t i) const -> auto { return indexer.from_index(i); }
  // access
  template <typename... Args>
  auto operator()(Args... is) -> T& {
    return data[indexer.get(is...)];
  }
  template <typename... Args>
  auto operator()(Args... is) const -> T const& {
    return data[indexer.get(is...)];
  }
  template <typename... Args>
  auto index(Args... is) const -> size_t {
    return indexer.get(is...);
  }
  auto dims() const -> auto { return indexer.dims(); }
};
