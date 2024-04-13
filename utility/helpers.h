/** send help
 * USAGE
 *  ???
 */
#pragma once

#include <cstddef>
#include <tuple>
#include <utility>

template <template <typename...> typename Template, typename... Args>
void is_specialization_of_impl(Template<Args...>);
template <typename Type, template <typename...> typename Template>
concept is_specialization_of = requires(Type __t) { is_specialization_of_impl<Template>(__t); };
