/* Min Stack
 * USAGE
 *  min_stack<T> arr(n);  optionally reserves for n items, default no reserve
 * MEMBERS
 *  size(), empty() as usual
 *  void push(T);
 *  void pop();
 *  T top();
 *  T min();
 * TIME
 *  O(1) push/pop/min/top
 * STATUS
 *  tested: cf/1938f
 */
#pragma once

#include <functional>
#include <vector>

template <typename T, typename Compare = std::less<>>
struct min_stack {
  struct stack_item {
    T value, min;
  };
  std::vector<stack_item> data;
  min_stack() = default;
  min_stack(int n) { data.reserve(n); }
  auto size() const -> size_t { return data.size(); }
  auto empty() const -> bool { return data.empty(); }
  auto min() const -> T { return data.back().min; }
  auto top() const -> T { return data.back().value; }
  auto pop() -> void { data.pop_back(); }
  auto push(T v) -> void {
    data.emplace_back(v, data.empty() ? v : std::min(v, data.back().min, Compare()));
  }
};
