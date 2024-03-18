/* Traits for binary search in binary search trees
 */
#pragma once

#include "utility/traits.h"

// clang-format off
MAKE_TRAITS(search_params,
  (LOWER_BOUND, UPPER_BOUND, FIND, BY_KEY, EMPLACE),
);
// clang-format on

namespace binary_search_details {
template <search_params params>
struct search_result {
  using type = bool;
};
template <search_params params>
  requires(params.has_all(params.FIND))
struct search_result<params> {
  using type = int_fast32_t;
};
template <search_params params>
using search_result_t = search_result<params>::type;

template <typename Key>
auto get_first(Key const& key, ...) -> decltype(auto) {
  return key;
}

template <search_params params, typename node_t, typename... Args>
  requires(
      not params.has_any(params.BY_KEY) and
      params.count(params.LOWER_BOUND | params.UPPER_BOUND | params.FIND) == 1)
inline auto search(node_t const& cur, Args const&... args) -> search_result_t<params> {
  if constexpr (params.has_all(params.UPPER_BOUND)) {
    return cur.search(args...) < 0;
  } else if constexpr (params.has_all(params.LOWER_BOUND)) {
    return cur.search(args...) <= 0;
  } else {
    return cur.search(args...);
  }
}

template <search_params params, typename node_t, typename Key>
  requires(
      params.has_any(params.BY_KEY) and
      params.count(params.LOWER_BOUND | params.UPPER_BOUND | params.FIND) == 1)
inline auto search(node_t const& cur, Key const& key) -> search_result_t<params> {
  if constexpr (params.has_all(params.UPPER_BOUND)) {
    return key < cur.key;
  } else if constexpr (params.has_all(params.LOWER_BOUND)) {
    return key <= cur.key;
  } else {
    return key < cur.key ? -1 : cur.key < key ? 1 : 0;
  }
}
}  // namespace binary_search_details
