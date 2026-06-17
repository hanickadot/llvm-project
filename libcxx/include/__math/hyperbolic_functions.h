//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___MATH_HYPERBOLIC_FUNCTIONS_H
#define _LIBCPP___MATH_HYPERBOLIC_FUNCTIONS_H

#include <__config>
#include <__type_traits/enable_if.h>
#include <__type_traits/is_integral.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

namespace __math {

// cosh

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 float cosh(float __x) _NOEXCEPT { return __builtin_coshf(__x); }

template <class = int>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double cosh(double __x) _NOEXCEPT {
  return __builtin_cosh(__x);
}

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 long double cosh(long double __x) _NOEXCEPT { return __builtin_coshl(__x); }

template <class _A1, __enable_if_t<is_integral<_A1>::value, int> = 0>
inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double cosh(_A1 __x) _NOEXCEPT {
  return __builtin_cosh((double)__x);
}

// sinh

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 float sinh(float __x) _NOEXCEPT { return __builtin_sinhf(__x); }

template <class = int>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double sinh(double __x) _NOEXCEPT {
  return __builtin_sinh(__x);
}

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 long double sinh(long double __x) _NOEXCEPT { return __builtin_sinhl(__x); }

template <class _A1, __enable_if_t<is_integral<_A1>::value, int> = 0>
inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double sinh(_A1 __x) _NOEXCEPT {
  return __builtin_sinh((double)__x);
}

// tanh

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 float tanh(float __x) _NOEXCEPT { return __builtin_tanhf(__x); }

template <class = int>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double tanh(double __x) _NOEXCEPT {
  return __builtin_tanh(__x);
}

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 long double tanh(long double __x) _NOEXCEPT { return __builtin_tanhl(__x); }

template <class _A1, __enable_if_t<is_integral<_A1>::value, int> = 0>
inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double tanh(_A1 __x) _NOEXCEPT {
  return __builtin_tanh((double)__x);
}

} // namespace __math

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___MATH_HYPERBOLIC_FUNCTIONS_H
