/* Traits for binary search in binary search trees
 */
#pragma once

#include "utility/traits.h"

// clang-format off
MAKE_TRAITS(search_params,
  (LOWER_BOUND, UPPER_BOUND, FIND, BY_KEY, EMPLACE, INSERT, GET_BOTH),
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

template <search_params params, typename Key>
  requires(params.count(params.INSERT) == 0)
auto get_key(Key const& key, ...) -> decltype(auto) {
  return key;
}
template <search_params params, typename Key>
  requires(params.count(params.INSERT) == 1)
auto get_key(auto const&, Key const& key, ...) -> decltype(auto) {
  return key;
}

template <search_params params, typename node_t, typename... Args>
  requires(params.count(params.INSERT) == 0)
auto get_search(node_t const& cur, Args const&... args) -> decltype(auto) {
  return cur.search(args...);
}
template <search_params params, typename node_t, typename... Args>
  requires(params.count(params.INSERT) == 1)
auto get_search(node_t const& cur, node_t const* data, auto const&, Args const&... args)
    -> decltype(auto) {
  return cur.search(data, args...);
}

constexpr auto valid_search_params(search_params params) -> bool {
  bool valid = true;
  valid &= params.count(params.LOWER_BOUND | params.UPPER_BOUND | params.FIND) == 1;
  if (params.has_any(params.EMPLACE)) {
    valid &= params.has_all(params.FIND | params.BY_KEY);
  }
  if (params.has_any(params.FIND)) {
    valid &= not params.has_any(params.INSERT | params.GET_BOTH);
  }
  return valid;
}

template <search_params params, typename node_t, typename... Args>
  requires(valid_search_params(params) and not params.has_any(params.BY_KEY))
inline auto search(node_t const& cur, Args const&... args) -> search_result_t<params> {
  if constexpr (params.has_all(params.UPPER_BOUND)) {
    return get_search<params>(cur, args...) < 0;
  } else if constexpr (params.has_all(params.LOWER_BOUND)) {
    return get_search<params>(cur, args...) <= 0;
  } else {
    return get_search<params>(cur, args...);
  }
}

template <search_params params, typename node_t, typename... Args>
  requires(valid_search_params(params) and params.has_any(params.BY_KEY))
inline auto search(node_t const& cur, Args const&... args) -> search_result_t<params> {
  auto const& key = get_key<params>(args...);
  if constexpr (params.has_all(params.UPPER_BOUND)) {
    return key < cur.key;
  } else if constexpr (params.has_all(params.LOWER_BOUND)) {
    return key <= cur.key;
  } else {
    return key < cur.key ? -1 : cur.key < key ? 1 : 0;
  }
}

#define _MAKE_SPLAY_DESCEND(direction) \
  template <search_params params, typename node_t, typename... Args> \
    requires( \
        valid_search_params(params) and not params.has_any(params.BY_KEY) and \
        not params.has_any(params.INSERT)) \
  inline auto descend_##direction(node_t const& cur, node_t const* data, Args&... args) { \
    if constexpr (requires { cur.descend_##direction(data, args...); }) { \
      cur.descend_##direction(data, args...); \
    } \
  } \
  template <search_params params, typename node_t, typename... Args> \
    requires( \
        valid_search_params(params) and not params.has_any(params.BY_KEY) and \
        params.has_any(params.INSERT)) \
  inline auto descend_##direction( \
      node_t const& cur, node_t const* data, auto const&, Args&... args) { \
    if constexpr (requires { cur.descend_##direction(data, args...); }) { \
      cur.descend_##direction(data, args...); \
    } \
  }
_MAKE_SPLAY_DESCEND(left)
_MAKE_SPLAY_DESCEND(right)
#undef _MAKE_SPLAY_DESCEND
}  // namespace binary_search_details
