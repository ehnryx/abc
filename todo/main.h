/** main template
 * Valid defines:
 *  PRINT_TIMING -- print timing after each SOLVE()
 *  MULTI_TEST -- multiple tests (`int T` is the first token of input)
 *  PRINT_CASE -- same as MULTI_TEST except also print "Case PRINT_CASE{i}: ans_i"
 *  FAST_INPUT -- use fast input
 *  FAST_INPUT_BUFFER -- size of buffer for fast_input. default=16384
 */
#pragma once

#if defined(PRINT_TIMING)
#include <chrono>
#endif
#include <iomanip>
#include <iostream>

template <typename input_t>
struct solve_main_wrapper {
  input_t& cin;
  solve_main_wrapper(input_t& _cin, int argc, char** argv) : cin(_cin) {
    (void)argc;
    (void)argv;
  }
  auto solve_main(int testnum) -> void;
  auto solve_all() -> int {
    int test_cases = 1;
#if defined(MULTI_TEST) or defined(PRINT_CASE)
    cin >> test_cases;
#endif
    for (int testnum = 1; testnum <= test_cases; testnum++) {
#define MAKE_STRING_IMPL(STRING) #STRING
#define MAKE_STRING(STRING) MAKE_STRING_IMPL(STRING)
#if defined(PRINT_CASE)
      std::cout << "Case " << MAKE_STRING(PRINT_CASE) << testnum << ": ";
#undef MAKE_STRING
#undef MAKE_STRING_IMPL
#endif
#if defined(PRINT_TIMING)
      auto start_time = std::chrono::high_resolution_clock::now();
#endif
      solve_main(testnum);
#if defined(PRINT_TIMING)
      auto duration = std::chrono::high_resolution_clock::now() - start_time;
      using namespace std::chrono;
      std::cerr << "\n[t" << testnum << "] " << duration / 1.0s << "s\n\n";
#endif
    }
    return 0;
  }
};

#if defined(FAST_INPUT)
#include "utility/fast_input.h"
#endif

auto main(int argc, char** argv) -> int {
  std::cout << std::fixed << std::setprecision(10);
#ifdef _USING_FAST_INPUT
#if not defined(FAST_INPUT_BUFFER)
#define FAST_INPUT_BUFFER 16384
#endif
  fast_input<FAST_INPUT_BUFFER> cin;
  solve_main_wrapper solver(cin, argc, argv);
#else
  std::cin.tie(0)->sync_with_stdio(0);
  solve_main_wrapper solver(std::cin, argc, argv);
#endif
  return solver.solve_all();
}

#define SOLVE() \
  template <typename input_t> \
  auto solve_main_wrapper<input_t>::solve_main([[maybe_unused]] int testnum) -> void

using ll = long long;
constexpr char nl = '\n';

#include <cassert>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;
