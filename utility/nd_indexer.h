/** N-Dimensional Indexer
 * USAGE
 *  nd_indexer<3> idx(n1, n2, n3);
 *  auto t = idx.get(i, j, k); // flattened index (row-major)
 *  auto [i, j, k] = idx.from_index(t); // gets the tuple
 *  idx.size() == n1 * n2 * n3; // total size
 * STATUS
 *  tested: cf/46e
 */
#pragma once

#include <cassert>
#include <stdexcept>
#include <tuple>

template <size_t ndims>
struct nd_indexer : nd_indexer<ndims - 1> {
  static_assert(ndims > 1);
  using nested_indexer = nd_indexer<ndims - 1>;
  size_t const _size;
  template <typename... Args>
  nd_indexer(size_t cur_dim, Args... ds)
      : nested_indexer(ds...), _size(nested_indexer::size() * cur_dim) {}
  auto size() const -> size_t { return _size; }
  template <typename... Args>
  auto get(size_t cur_i, Args... is) const -> size_t {
    return cur_i * nested_indexer::size() + nested_indexer::get(is...);
  }
  auto from_index(size_t i) const -> auto {
    size_t cur_i = i / nested_indexer::size();
    return std::tuple_cat(
        std::tuple(cur_i), nested_indexer::from_index(i - nested_indexer::size() * cur_i));
  }
  auto dims() const -> auto {
    return std::tuple_cat(std::tuple(size() / nested_indexer::size()), nested_indexer::dims());
  }
};

template <>
struct nd_indexer<1> {
  size_t const _size;
  nd_indexer(size_t dim, ...) : _size(dim) {}
  auto size() const -> size_t { return _size; }
  auto get(size_t i) const -> size_t { return i; }
  auto from_index(size_t i) const -> auto { return std::tuple(i); }
  auto dims() const -> auto { return std::tuple(size()); }
};
