/**
 * Random helper
 */
#pragma once

#include <random>

#if not defined(RANDOM_ENGINE)
#define RANDOM_ENGINE std::mt19937
#endif

inline auto get_rng() -> RANDOM_ENGINE& {
  static auto rng = [] {
#ifdef RANDOM_SEED
    return RANDOM_ENGINE(RANDOM_SEED);
#else
    std::random_device rd;
    return RANDOM_ENGINE(rd());
#endif
  }();
  return rng;
}
