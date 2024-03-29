/* Polynomial String Hashing
 * USAGE
 *  hash_multi<hash_single, ...> hasher(string_s)
 * MEMBERS
 *  get(l, r); returns tuple of hashes
 * TIME
 *  everything dispatches to the component hashes
 * STATUS
 *  tested: cf/1326d2
 */
#pragma once

#include <tuple>

template <typename... HashSingle>
struct hash_multi : HashSingle... {
  template <typename Container>
  hash_multi(Container const& s) : HashSingle(s)... {}
  auto get(int i, int len) -> auto { return std::make_tuple(HashSingle::get(i, len)...); }
};
