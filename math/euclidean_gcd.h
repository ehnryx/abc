/* Extended Euclidean Algorithm
 * USAGE
 *  auto [gcd, x, y] = extended_gcd(a, b);
 *  returns gcd, x and y such that ax + by = gcd
 * STATUS
 *  tested via numbers/mod_int.h: mod_int.inverse
 */
#pragma once

#include <tuple>

template <std::integral T>
constexpr auto euclidean_gcd(T a, T b) -> std::tuple<T, T, T> {
  if (b == 0) return std::tuple(a, 1, 0);
  auto [g, y, x] = euclidean_gcd(b, a % b);
  return std::tuple(g, x, y - x * (a / b));
}
