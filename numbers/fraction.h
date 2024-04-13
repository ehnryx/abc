/* Fraction
 * USAGE
 *  fraction<T> f(numerator, denominator);
 *  1/0 is +inf, -1/0 is -inf, 0/0 is treated as NaN
 * STATUS
 *  tested: boj/30855
 */
#pragma once

#include <numeric>
#include <ostream>

namespace fraction_details {
struct skip_gcd {};
template <typename T>
auto gcd(T const& a, T const& b) {
  if constexpr (std::is_integral_v<T>) return std::abs(std::gcd(a, b));
  else {
    if (b == 0) return a < 0 ? -a : a;
    return gcd(b, a % b);
  }
}
};  // namespace fraction_details

template <typename T>
struct fraction {
  T num, den;
  fraction(T const& n = 0) : num(n), den(1) {}
  fraction(T const& n, T const& d) {
    T const g = fraction_details::gcd(n, d);
    num = d < 0 ? -n / g : n / g;
    den = d < 0 ? -d / g : d / g;
  }
  fraction(T const& n, T const& d, fraction_details::skip_gcd) : num(n), den(d) {}
  static auto inf() -> fraction { return fraction(1, 0, fraction_details::skip_gcd{}); }
  static auto nan() -> fraction { return fraction(0, 0, fraction_details::skip_gcd{}); }
  template <typename U>
  fraction(fraction<U> const& o) : num(o.num), den(o.den) {}
  auto numerator() const -> T const& { return num; }
  auto denominator() const -> T const& { return den; }
  auto floor() const -> T { return num < 0 ? (num - den + 1) / den : num / den; }
  auto integer_part() const -> T { return floor(); }
  auto fractional_part() const -> fraction {
    T rem = num % den;
    return fraction(rem < 0 ? rem + den : rem, den, fraction_details::skip_gcd{});
  }
  friend std::ostream& operator<<(std::ostream& os, fraction const& v) {
    return os << v.numerator() << '/' << v.denominator();
  }
  fraction operator-() const { return fraction(-num, den, fraction_details::skip_gcd{}); }
  fraction operator+(fraction const& o) const { return fraction(*this) += o; }
  fraction operator-(fraction const& o) const { return fraction(*this) -= o; }
  fraction operator*(fraction const& o) const { return fraction(*this) *= o; }
  fraction operator/(fraction const& o) const { return fraction(*this) /= o; }
  // clang-format off
  template <std::integral U>
  friend auto operator*(U const& c, fraction const& f) -> fraction { return f * c; }
  template <std::integral U>
  friend auto operator/(U const& c, fraction const& f) -> fraction { return f.inverse() * c; }
  // clang-format on
  fraction& operator+=(fraction const& o) {
    T g = fraction_details::gcd(den, o.den);
    den /= g;
    num = num * (o.den / g) + o.num * den;
    g = fraction_details::gcd(num, g);
    num /= g;
    den *= o.den / g;
    return *this;
  }
  fraction& operator-=(fraction const& o) {
    T g = fraction_details::gcd(den, o.den);
    den /= g;
    num = num * (o.den / g) - o.num * den;
    g = fraction_details::gcd(num, g);
    num /= g;
    den *= o.den / g;
    return *this;
  }
  fraction& operator*=(fraction const& o) {
    T const gn = fraction_details::gcd(num, o.den);
    T const gd = fraction_details::gcd(den, o.num);
    num = num / gn * o.num / gd;
    den = den / gd * o.den / gn;
    return *this;
  }
  fraction& operator/=(fraction const& o) {
    T const gn = fraction_details::gcd(num, o.num);
    T const gd = fraction_details::gcd(den, o.den);
    num = num / gn * (o.num < 0 ? -o.den : o.den) / gd;
    den = den / gd * (o.num < 0 ? -o.num : o.num) / gn;
    return *this;
  }
  auto inverse() const -> fraction {
    return num < 0 ? fraction(-den, -num, fraction_details::skip_gcd{})
                   : fraction(den, num, fraction_details::skip_gcd{});
  }
  /// may overflow
  auto operator<(fraction const& o) const -> bool {
    if (den == 0 && o.den == 0) return num && o.num && num < o.num;
    if (den == 0) return num < 0;
    if (o.den == 0) return 0 < o.num;
    return num * o.den < o.num * den;
  }
  /// operator< but guards against overflow
  auto compare(fraction const& o) const -> bool {
    if (den == 0 && o.den == 0) return num && o.num && num < o.num;
    if (den == 0) return num < 0;
    if (o.den == 0) return 0 < o.num;
    return fraction(*this).compare_guard_impl(o);
  }
  /// destroys *this
  auto compare_guard_impl(fraction o) -> bool {
    while (den != o.den) {
      T y = floor();
      T oy = o.floor();
      if (y != oy) return y < oy;
      num -= den * y;
      o.num -= o.den * oy;
      if (num == 0 || o.num == 0) break;
      if ((den > o.den && 2 * num > den) || (o.den > den && 2 * o.num > o.den)) {
        num = den - num;
        o.num = o.den - o.num;
      } else {
        swap(num, den);
        swap(o.num, o.den);
      }
      swap(num, o.num);
      swap(den, o.den);
    }
    return num < o.num;
  }
  auto operator>(fraction const& o) const -> bool { return o < *this; }
  auto operator==(fraction const& o) const -> bool { return num == o.num && den == o.den; }
  auto operator<=(fraction const& o) const -> bool { return operator==(o) || operator<(o); }
  auto operator>=(fraction const& o) const -> bool { return o <= *this; }
  template <typename D>
  auto value() const -> D {
    return D(num) / D(den);
  }
  explicit operator float() const { return value<float>(); }
  explicit operator double() const { return value<double>(); }
  explicit operator long double() const { return value<long double>(); }
  auto abs() const -> fraction {
    return fraction(num < 0 ? -num : num, den, fraction_details::skip_gcd{});
  }
  struct compare_as_pair {
    auto operator()(fraction const& a, fraction const& b) const -> bool {
      return a.num < b.num || (a.num == b.num && a.den < b.den);
    }
  };
};

template <typename T>
auto abs(fraction<T> const& v) -> fraction<T> {
  return v.abs();
}
