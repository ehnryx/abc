/** send help
 * USAGE
 *  ???
 */
#pragma once

#include <cstddef>
#include <tuple>

template <template <typename...> typename Template, typename... Args>
void is_specialization_of_impl(Template<Args...>);
template <typename Type, template <typename...> typename Template>
concept is_specialization_of = requires(Type __t) { is_specialization_of_impl<Template>(__t); };

template <size_t Index, typename... Args>
auto get_from_pack(Args&&... args) -> decltype(auto) {
  return std::get<Index>(std::forward_as_tuple(args...));
}
template <size_t Index, typename... Args>
using get_from_pack_t = decltype(get_from_pack<Index>(std::declval<Args>()...));
