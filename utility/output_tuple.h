/** Helper for fast input
 * USAGE
 *  gives easy output for pairs/tuples
 * STATUS
 *  untested
 */
#pragma once

#include <ostream>
#include <tuple>

template <typename... T>
  requires(sizeof...(T) > 0)
struct output_tuple : std::tuple<T...> {
  constexpr output_tuple(T const&... ts) : std::tuple<T...>(ts...) {}
  constexpr output_tuple(std::pair<T...> const& p) : std::tuple<T...>(p.first, p.second) {}
  constexpr output_tuple(std::tuple<T...> const& t) : std::tuple<T...>(t) {}
  template <size_t... Is>
  friend auto print_helper(std::ostream& os, output_tuple const& v, std::index_sequence<Is...>)
      -> std::ostream& {
    return (((os << get<0>(v)) << " " << std::get<Is + 1>(v)), ...);
  }
  friend auto operator<<(std::ostream& os, output_tuple const& v) -> std::ostream& {
    return print_helper(os, v, std::make_index_sequence<sizeof...(T) - 1>());
  }
};