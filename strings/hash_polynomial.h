/* Polynomial String Hashing
 * USAGE
 *  hash_polynomial<{prime, mod}> hasher(string_s)
 *  prime, mod are ints
 * INPUT
 *  some container that is iterable with size
 *  maps s_i -> s_i * prime^i % mod, outputs range sum
 * MEMBERS
 *  get(l, r); returns (sum_{l <= i < r} s_i * prime^i) % mod
 *  operator hash_t(); returns hash of the full string
 * TIME
 *  O(N) precomputation, O(1) query
 *  N = |string|
 * STATUS
 *  tested: cf/1326d2
 *  untested: nadc21/b
 */
#pragma once

#include "numbers/mod_int.h"

#include <vector>

struct hash_params {
  int mod, multiplier;
};

//template <hash_params params>
template <hash_params params>
struct hash_polynomial {
  using hash_t = decltype(std::declval<mod_int<params.mod>>().value());
  std::vector<mod_int<params.mod>> sum, invm;
  template <typename Container>
  hash_polynomial(Container const& s) : sum(s.size() + 1), invm(s.size() + 1) {
    auto m_inverse = mod_int<params.mod>(params.multiplier).inverse();
    auto x = invm[0] = mod_int<params.mod>(1);
    for (size_t i = 0; i < s.size(); i++) {
      x *= params.multiplier;
      sum[i + 1] = sum[i] + x * s[i];
      invm[i + 1] = invm[i] * m_inverse;
    }
  }
  auto get(int i, int len) const -> hash_t {
    return ((sum[i + len] - sum[i]) * invm[i]).value();
  }
  operator hash_t() const { sum.back().value(); }
};
