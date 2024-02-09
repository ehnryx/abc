/** Helper for fast input
 * USAGE
 *  specialize and define `get` as follows
 *  template <>
 *  struct fast_input_read<T> {
 *    static auto get(auto& input, Int& value) -> void;
 *  };
 * STATUS
 *  untested
 */
#pragma once

#include <cstddef>

template <typename T = void>
struct fast_input_read {};
