/* Opt Tracker
 * USAGE
 *  keeps track of best O(1) values (min or max)
 * STATUS
 *  tested: cf/102787d
 */
#pragma once

#include <functional>
#include <limits>

template <typename T, size_t N, typename Compare = std::less<>>
  requires(N >= 1)
struct opt_tracker {
  std::array<T, N> data;
  opt_tracker(T default_value) { clear(default_value); }
  auto clear(T default_value) -> void { std::fill(data.begin(), data.end(), default_value); }
  auto operator[](int i) -> T& { return data[i]; }
  auto operator[](int i) const -> T const& { return data[i]; }
  auto push(T x) -> void {
    for (size_t i = 0; i < N; i++) {
      if (Compare()(x, data[i])) {
        std::swap(x, data[i]);
      }
    }
  }
};

template <typename T, size_t N, typename Compare = std::less<>>
  requires(N >= 1)
struct opt_tracker_count {
  struct key_count {
    T value;
    size_t count;
  };
  std::array<key_count, N> data;
  opt_tracker_count(T default_value) { clear(default_value); }
  auto clear(T default_value) -> void {
    std::fill(data.begin(), data.end(), key_count{.value = default_value, .count = 0});
  }
  auto operator[](int i) -> key_count& { return data[i]; }
  auto operator[](int i) const -> key_count const& { return data[i]; }
  auto value(int i) const -> T const& { return data[i].value; }
  auto count(int i) const -> size_t { return data[i].count; }
  auto push(key_count const& item) -> void { return push(item.value, item.count); }
  auto push(T x, size_t mult = 1) -> void {
    for (size_t i = 0; i < N; i++) {
      if (x == data[i].value) {
        data[i].count += mult;
        return;
      }
      if (Compare()(x, data[i].value)) {
        std::swap(x, data[i].value);
        std::swap(mult, data[i].count);
      }
    }
  }
};
