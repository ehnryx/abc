/* Bump Allocator
 * Source: https://github.com/kth-competitive-programming/kactl
 * USAGE
 *  EXAMPLES
 *    set<key, bump_alloc<key>>;
 *    map<key, bump_alloc<pair<const key, value>>>;
 *    vector<key, bump_alloc<key>>;
 *  set buffer size by setting BUMP_ALLOC_SIZE
 * STATUS
 *  untested
 */
#pragma once

#include <cstddef>

#if not defined(BUMP_ALLOC_SIZE)
#define BUMP_ALLOC_SIZE 64 << 20
#endif

namespace bump_alloc_details {
alignas(16) char buffer[BUMP_ALLOC_SIZE];
size_t buffer_idx = sizeof(buffer);
template <typename T>
auto allocate(size_t n) -> T* {
  buffer_idx = (buffer_idx - n * sizeof(T)) & -alignof(T);
  return reinterpret_cast<T*>(buffer + buffer_idx);
}
}  // namespace bump_alloc_details

template <typename T>
struct bump_alloc {
  using value_type = T;
  bump_alloc() = default;
  template <typename U>
  bump_alloc(U const&) {}
  auto allocate(size_t n) -> T* { return bump_alloc_details::allocate<T>(n); }
  auto deallocate(T*, size_t) -> void {}
};
