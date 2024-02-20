/** class Member checker
 * USAGE
 *  MEMBER_TYPE_CHECKER(type_name);
 *  using T = _has_type_{type_name}<class_type>::type;
 *  bool exists = _has_type_{type_name}<class_type>::value;
 */
#pragma once

#define MEMBER_TYPE_CHECKER(TYPE) \
  template <typename __class_t> \
  struct _has_type_##TYPE { \
    using type = void; \
    static constexpr bool value = false; \
  }; \
  template <typename __class_t> \
    requires(requires { typename __class_t::TYPE; }) \
  struct _has_type_##TYPE<__class_t> { \
    using type = __class_t::TYPE; \
    static constexpr bool value = true; \
  }

#define MEMBER_VARIABLE_CHECKER(VARIABLE) \
  template <typename __class_t> \
  struct _has_variable_##VARIABLE { \
    static constexpr bool value = false; \
    using type = void; \
  }; \
  template <typename __class_t> \
    requires(requires { __class_t::VARIABLE; }) \
  struct _has_variable_##VARIABLE<__class_t> { \
    static constexpr bool value = true; \
    using type = decltype(__class_t::VARIABLE); \
  }

#define MEMBER_FUNCTION_CHECKER(FUNCTION) \
  template <typename __class_t, typename... __args_t> \
  struct _has_function_##FUNCTION { \
    static constexpr bool value = false; \
    using type = void; \
  }; \
  template <typename __class_t, typename... __args_t> \
    requires(requires(__class_t __class, __args_t... __args) { __class.FUNCTION(__args...); }) \
  struct _has_function_##FUNCTION<__class_t, __args_t...> { \
    static constexpr bool value = true; \
    using type = decltype(std::declval<__class_t>().FUNCTION(std::declval<__args_t>()...)); \
  }
