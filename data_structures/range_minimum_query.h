/* Range Minimum Query
 * USAGE
 *  data_structures/sparse_table.h
 */
#pragma once

#include "data_structures/sparse_table.h"

#include <functional>

namespace range_minimum_query_details {
template <typename Compare>
struct functional_compare {
  template <typename T, typename U>
  auto operator()(T const& left, U const& right) -> auto {
    return Compare()(left, right) ? left : right;
  }
};
}  // namespace range_minimum_query_details

template <typename T, typename Compare = std::less<T>>
using range_minimum_query =
    sparse_table<T, range_minimum_query_details::functional_compare<Compare>>;
