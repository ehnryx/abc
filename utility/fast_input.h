/** Fast Input
 * USAGE
 *  fast_input<buf_size> cin;
 *  cin >> blah;
 * STATUS
 *  tested: cf/46e (positive and negative ints. 280 -> 77 vs std::cin)
 */
#pragma once

#include "utility/fast_input_read.h"

#include <complex>
#include <string>
#include <tuple>

#define USING_FAST_INPUT

template <size_t buf_size>
struct fast_input {
  char buf[buf_size], *S, *T;
  FILE* const ifptr;
  fast_input(FILE* _in = stdin) : S(buf), T(buf), ifptr(_in) {}

  static auto is_digit(char c) -> bool { return '0' <= c and c <= '9'; }

  explicit operator bool() const { return peek() != EOF; }

  template <typename T>
  auto operator>>(T& x) -> fast_input& {
    if constexpr (requires(T& t) { this->get(t); }) {
      this->get(x);
    } else {
      fast_input_read<T>::get(*this, x);
    }
    return *this;
  }

  auto getc() -> char {
    if (S == T) {
      T = (S = buf) + fread(buf, 1, buf_size, ifptr);
      if (S == T) return EOF;
    }
    return *S++;
  }

  auto peek() -> char {
    if (S == T) {
      T = (S = buf) + fread(buf, 1, buf_size, ifptr);
      if (S == T) return EOF;
    }
    return *S;
  }

  auto get(char& x) -> void {
    while (isspace(x = getc()) && x != EOF) {}
  }

  // TODO slow?
  auto get(std::string& x) -> void {
    x.clear();
    char c;
    while (isspace(c = getc()) && c != EOF) {}
    for (; !isspace(c) && c != EOF; c = getc()) {
      x.push_back(c);
    }
  }

  auto get(decltype(std::ignore)) -> void {
    char c;
    while (isspace(c = getc()) && c != EOF) {}
    for (; !isspace(c) && c != EOF; c = getc()) {}
  }

  template <typename var_t>
    requires(std::is_integral_v<var_t>)
  auto get(var_t& x) -> void {
    x = 0;
    char c;
    bool negative = false;
    while (!is_digit(c = getc()) && c != EOF) {
      negative = (c == '-');
    }
    for (; is_digit(c) && c != EOF; c = getc()) {
      x = x * 10 + c - '0';
    }
    if (negative) {
      x = -x;
    }
  }

  // TODO slow ?
  template <typename var_t>
    requires(std::is_floating_point_v<var_t>)
  auto get(var_t& x) -> void {
    x = 0;
    char c;
    bool negative = false;
    while (!is_digit(c = getc()) && c != '.' && c != EOF) {
      negative = (c == '-');
    }
    if (c != '.') {
      for (; is_digit(c) && c != EOF; c = getc()) {
        x = x * 10 + c - '0';
      }
    }
    if (c == '.') {
      static var_t div;
      div = 1;
      while (is_digit(c = getc()) && c != EOF) {
        x = x * 10 + c - '0';
        div *= 10;
      }
      x /= div;
    }
    if (negative) {
      x = -x;
    }
  }

  template <typename T, typename U>
  auto get(std::pair<T, U>& x) -> void {
    *this >> x.first >> x.second;
  }

  template <size_t index = 0, typename... T>
  inline auto get(std::tuple<T...>& x) -> void {
    if constexpr (index < sizeof...(T)) {
      *this >> (std::get<index>(x));
      get<index + 1>(x);
    }
  }

  template <typename T>
  auto get(std::complex<T>& x) -> void {
    T a, b;
    *this >> a >> b;
    x = {a, b};
  }

  auto getline() -> std::string {
    std::string out;
    char c;
    while ((c = getc()) != '\n' && c != EOF) {
      out.push_back(c);
    }
    return out;
  }
};
