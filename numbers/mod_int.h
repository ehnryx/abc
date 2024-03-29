/* Mod Int
 * USAGE
 *  mod_int<mod> v(num);
 *  mod needs to fit in a 32-bit integer
 * NOTES
 *  division `operator/` is slow
 * STATUS
 *  tested: cf/1540b,1548c; boj/14517,11726
 */
#pragma once

#include "math/euclidean_gcd.h"

#include <cstdint>
#include <iostream>

template <int32_t mod_value>
  requires(mod_value > 0)
struct mod_int {
  using mod_t = int_fast32_t;
  using wide_t = int_fast64_t;
  static constexpr mod_t mod = mod_value;

  mod_t v;
  mod_int() : v(0) {}
  template <typename T>
  mod_int(T c) : v(c % mod) {
    if (v < 0) v += mod;
  }
  friend auto operator>>(std::istream& is, mod_int& num) -> std::istream& {
    wide_t val;
    is >> val;
    num = mod_int(val);
    return is;
  }
  friend auto operator<<(std::ostream& os, const mod_int& num) -> std::ostream& {
    return os << num.v;
  }
  auto value() const -> mod_t { return v; }
  auto readable_value() const -> mod_t { return 2 * v <= mod ? v : v - mod; }
  explicit operator bool() const { return v != 0; }

  auto operator==(mod_int const& o) const -> bool { return v == o.v; }
  auto operator-() const -> mod_int { return mod_int(*this).negate(); }
  auto operator+(mod_int const& o) const -> mod_int { return mod_int(*this) += o; }
  auto operator-(mod_int const& o) const -> mod_int { return mod_int(*this) -= o; }
  auto operator*(mod_int const& o) const -> mod_int { return mod_int(*this) *= o; }
  auto operator/(mod_int const& o) const -> mod_int { return mod_int(*this) /= o; }
  auto negate() -> mod_int& {
    if (v != 0) v = mod - v;
    return *this;
  }
  auto operator+=(const mod_int& o) -> mod_int& {
    v += o.v;
    if (v >= mod) v -= mod;
    return *this;
  }
  auto operator-=(const mod_int& o) -> mod_int& {
    v -= o.v;
    if (v < 0) v += mod;
    return *this;
  }
  auto operator*=(const mod_int& o) -> mod_int& {
    v = mod_t(wide_t(v) * wide_t(o.v) % wide_t(mod));
    return *this;
  }
  auto operator/=(const mod_int& o) -> mod_int& { return operator*=(o.inverse()); }
  auto inverse() const -> mod_int {
    auto [g, x, y] = euclidean_gcd(mod, v);
    if (g != 1) throw std::invalid_argument("taking the inverse of a non-coprime number");
    return mod_int(y);
  }
  auto pow(int_fast64_t exponent) const -> mod_int {
    if (exponent == 0) return mod_int(1);
    if (v == 0) {
      if (exponent < 0) throw std::invalid_argument("raising zero to a negative power");
      return mod_int(0);
    }
    auto res = mod_int(1);
    auto base = mod_int(exponent < 0 ? inverse() : *this);
    for (; exponent != 0; exponent /= 2) {
      if (exponent % 2 != 0) res *= base;
      base *= base;
    }
    return res;
  }
};

#ifdef FAST_INPUT
#include "utility/fast_input_read.h"
template <int_fast32_t mod>
struct fast_input_read<mod_int<mod>> {
  template <typename input_t>
  static auto get(input_t& in, mod_int<mod>& num) -> void {
    typename mod_int<mod>::wide_t val;
    in >> val;
    num = mod_int<mod>(val);
  }
};
#endif
