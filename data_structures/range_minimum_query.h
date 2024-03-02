/* Range Minimum Query
 * USAGE
 *  data_structures/sparse_table.h
 */
#pragma once

#include "data_structures/sparse_table.h"
#include "utility/helpers.h"

#include <functional>

template <typename T, typename Compare = std::less<T>>
using range_minimum_query = sparse_table<T, make_functional<Compare>>;
