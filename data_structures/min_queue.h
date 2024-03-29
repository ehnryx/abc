/* Min Queue
 * USAGE
 *  min_queue<T> arr(n);  optionally reserves for n items, default no reserve
 * MEMBERS
 *  size(), empty() as usual
 *  void push(T);
 *  void pop();
 *  T front();
 *  T min();
 * TIME
 *  O(1) push/pop/min/front
 * STATUS
 *  tested: cf/1938f
 */
#pragma once

#include "data_structures/min_stack.h"

template <typename T, typename Compare = std::less<>>
struct min_queue {
  min_stack<T, Compare> in, out;
  min_queue() = default;
  min_queue(int n) : in(n), out(n) {}
  auto size() const -> size_t { return in.size() + out.size(); }
  auto empty() const -> bool { return in.empty() and out.empty(); }
  auto min() const -> T {
    if (in.empty()) return out.min();
    if (out.empty()) return in.min();
    return std::min(in.min(), out.min(), Compare());
  }
  auto top() -> T {
    if (out.empty()) _refill();
    return out.top();
  }
  auto pop() -> void {
    if (out.empty()) _refill();
    out.pop();
  }
  auto push(T v) -> void { in.push(v); }
  auto _refill() -> void {
    while (!in.empty()) {
      out.push(in.top());
      in.pop();
    }
  }
};
