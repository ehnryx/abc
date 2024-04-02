/* Golden-section Search
 * STATUS
 *  tested: boj/10785,11662,19128
 */
#pragma once

#include <functional>
#include <numbers>
#include <optional>

template <typename Result, typename Coordinate>
struct search_result {
  Result value;
  Coordinate index;
  operator Result() const { return value; }
};

/// Compare should return true if the first is better
template <std::floating_point Coordinate, typename Function, typename Compare = std::less<>>
auto golden_section_search(
    Coordinate left, Coordinate right, Function const& f, Compare const& cmp = std::less<>(),
    int n_iters = 90) -> search_result<decltype(f(left)), Coordinate> {
  static constexpr auto inv_phi = Coordinate(1) / std::numbers::phi_v<Coordinate>;
  std::optional<decltype(f(left))> l_value, r_value;
  for (int iteration = 0; iteration < n_iters; iteration++) {
    auto const l_mid = left * inv_phi + right * (1 - inv_phi);
    auto const r_mid = left * (1 - inv_phi) + right * inv_phi;
    if (not l_value) l_value.emplace(f(l_mid));
    if (not r_value) r_value.emplace(f(r_mid));
    if (cmp(*l_value, *r_value)) {
      right = r_mid;
      r_value = std::move(l_value);
      l_value.reset();
    } else {
      left = l_mid;
      l_value = std::move(r_value);
      r_value.reset();
    }
  }
  return search_result{
      .value = l_value ? std::move(*l_value) : std::move(*r_value),
      .index = l_value ? left * inv_phi + right * (1 - inv_phi)
                       : left * (1 - inv_phi) + right * inv_phi,
  };
}
