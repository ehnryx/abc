/* Named Types
 * USAGE
 *  so that f(true, true, false); is less confusing
 */
#pragma once

#include <concepts>

template <std::floating_point T>
struct epsilon {
  T const value;
  constexpr operator T() const { return value; }
};

#define MAKE_NAMED_BOOL(NAME) \
  struct NAME { \
    bool const value; \
    constexpr operator bool() const { \
      return value; \
    } \
  };
MAKE_NAMED_BOOL(strict);
MAKE_NAMED_BOOL(keep);
#undef MAKE_NAMED_BOOL
