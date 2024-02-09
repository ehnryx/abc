/** N-Dimensional Array
 * USAGE
 *  nd_array<int, 3> arr(n1, n2, n3, initial_value);
 *  nd_array<int, 3> arr(n1, n2, n3);
 *  arr(i, j, k) for access (instead of arr[i][j][k])
 * NOTES
 *  stored in row-major order in a flattened vector
 *  unable to resize
 * STATUS
 *  untested
 */
#pragma once

#include "utility/nd_indexer.h"

#include <vector>

template <typename T, size_t dims>
struct nd_array {
  const nd_indexer<dims> indexer;
  std::vector<T> data;
  template <typename... Args>
    requires(sizeof...(Args) == dims)
  nd_array(Args... ds) : indexer(ds...), data(indexer.size()) {}
  template <typename... Args>
    requires(sizeof...(Args) == dims + 1)
  nd_array(Args... ds) : indexer(ds...), data(indexer.size(), (ds, ...)) {}
  size_t size() const { return indexer.size(); }
  T& operator[](size_t i) { return data[i]; }
  const T& operator[](size_t i) const { return data[i]; }
  auto from_index(size_t i) const { return indexer.from_index(i); }
  // access
  template <typename... Args>
  T& operator()(Args... is) {
    return data[indexer.get(is...)];
  }
  template <typename... Args>
  const T& operator()(Args... is) const {
    return data[indexer.get(is...)];
  }
  template <typename... Args>
  size_t index(Args... is) const {
    return indexer.get(is...);
  }
};
