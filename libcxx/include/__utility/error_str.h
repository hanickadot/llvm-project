//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___UTILITY_ERROR_STR_H
#define _LIBCPP___UTILITY_ERROR_STR_H

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

#if _LIBCPP_STD_VER >= 26

[[noreturn]] _LIBCPP_HIDE_FROM_ABI constexpr void constexpr_error_str(const char * msg) {
  __constexpr_error(msg);
}

#endif

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___UTILITY_ERROR_STR_H
