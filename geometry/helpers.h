#pragma once

#include <concepts>

namespace geometry {
template <typename T>
concept non_floating = not std::is_floating_point_v<T>;

template <typename T>
struct default_point_traits {
  static constexpr bool not_implemented = true;
};
template <std::floating_point T>
struct default_point_traits<T> {
  using product_t = T;
  using intersection_t = T;
};
template <typename T>
  requires(std::is_integral_v<T> and sizeof(T) <= 4)
struct default_point_traits<T> {
  using product_t = long long;
  using intersection_t = double;
};
template <typename T>
  requires(std::is_integral_v<T> and 4 < sizeof(T) and sizeof(T) <= 8)
struct default_point_traits<T> {
  using product_t = __int128;
  using intersection_t = long double;
};
template <>
struct default_point_traits<__int128> {
  using product_t = __int128;  // only used for delaunay
  using intersection_t = long double;
};

template <typename T>
concept integral_with_128 = std::is_integral_v<T> or std::is_same_v<T, __int128>;
template <typename T, typename From>
struct implicit_conversion {
  static constexpr bool value =
      std::is_floating_point_v<T> or
      (integral_with_128<T> and integral_with_128<From> and sizeof(From) <= sizeof(T));
};
template <typename T, typename From>
constexpr bool implicit_conversion_v = implicit_conversion<T, From>::value;
}  // namespace geometry

#include "utility/named_types.h"

namespace geometry {
template <std::floating_point T>
int sign(epsilon<T> eps, T x) {
  return x < -eps ? -1 : x > eps ? 1 : 0;
}
template <non_floating T>
int sign(T x) {
  return x < 0 ? -1 : x > 0 ? 1 : 0;
}
}  // namespace geometry
