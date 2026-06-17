//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___MATH_GAMMA_H
#define _LIBCPP___MATH_GAMMA_H

#include <__config>
#include <__type_traits/enable_if.h>
#include <__type_traits/is_integral.h>
#include <__type_traits/is_constant_evaluated.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

namespace __math {

// lgamma

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 float lgamma(float __x) _NOEXCEPT { return __builtin_lgammaf(__x); }

template <class = int>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double lgamma(double __x) _NOEXCEPT {
  return __builtin_lgamma(__x);
}

inline _LIBCPP_HIDE_FROM_ABI  _LIBCPP_CONSTEXPR_SINCE_CXX23 long double lgamma(long double __x) _NOEXCEPT { return __builtin_lgammal(__x); }

template <class _A1, __enable_if_t<is_integral<_A1>::value, int> = 0>
inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double lgamma(_A1 __x) _NOEXCEPT {
  return __builtin_lgamma((double)__x);
}

// nan

// tgamma

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 float tgamma(float __x) _NOEXCEPT { return __builtin_tgammaf(__x); }

template <class = int>
_LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double tgamma(double __x) _NOEXCEPT {
  return __builtin_tgamma(__x);
}

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 long double tgamma(long double __x) _NOEXCEPT { return __builtin_tgammal(__x); }

template <class _A1, __enable_if_t<is_integral<_A1>::value, int> = 0>
inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double tgamma(_A1 __x) _NOEXCEPT {
  return __builtin_tgamma((double)__x);
}

// __lgamma_r
//
// POSIX systems provide a function named lgamma_r which is a reentrant version of lgamma. Use that
// whenever possible. However, we avoid re-declaring the actual function since different platforms
// declare it differently in the first place: instead use `asm` to get the compiler to call the right
// function.

#if defined(_LIBCPP_MSVCRT_LIKE) // reentrant version is not available on Windows

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double __lgamma_r(double __d) _NOEXCEPT { return __builtin_lgamma(__d); }

#else

#  if defined(_LIBCPP_OBJECT_FORMAT_MACHO)
double __lgamma_r_shim(double, int*) _NOEXCEPT __asm__("_lgamma_r");
#  else
double __lgamma_r_shim(double, int*) _NOEXCEPT __asm__("lgamma_r");
#  endif

inline _LIBCPP_HIDE_FROM_ABI _LIBCPP_CONSTEXPR_SINCE_CXX23 double __lgamma_r(double __d) _NOEXCEPT {
  int __sign;
  if (__libcpp_is_constant_evaluated()) {
    return __builtin_lgamma(__d);
  } else {
    return __math::__lgamma_r_shim(__d, &__sign);
  }
}

#endif

} // namespace __math

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___MATH_GAMMA_H
