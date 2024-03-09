/* Bump Allocator
 * USAGE
 *  EXAMPLES
 *    set<key, bump_allocator<key>>;
 *    map<key, bump_allocator<pair<const key, value>>>;
 *    vector<key, bump_allocator<key>>;
 *  set buffer size by setting BUMP_ALLOC_SIZE
 * STATUS
 *  untested
 */
#pragma once

#include <cstddef>

#if not defined(BUMP_ALLOCATOR_SIZE)
#define BUMP_ALLOCATOR_SIZE 64 << 20
#endif

namespace bump_allocator_details {
alignas(16) std::byte buffer[BUMP_ALLOCATOR_SIZE];
size_t buffer_idx = sizeof(buffer);
template <typename T>
auto allocate(size_t n) -> T* {
  buffer_idx = (buffer_idx - n * sizeof(T)) / alignof(T) * alignof(T);
  return reinterpret_cast<T*>(buffer + buffer_idx);
}
}  // namespace bump_allocator_details

template <typename T>
struct bump_allocator {
  using value_type = T;
  bump_allocator() = default;
  template <typename U>
  bump_allocator(U const&) {}
  auto allocate(size_t n) -> T* { return bump_allocator_details::allocate<T>(n); }
  auto deallocate(T*, size_t) -> void {}
};
