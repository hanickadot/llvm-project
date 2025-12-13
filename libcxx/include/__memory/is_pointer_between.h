// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___MEMORY_IS_POINTER_BETWEEN_H
#define _LIBCPP___MEMORY_IS_POINTER_BETWEEN_H

#include <__config>
#include <__cstddef/size_t.h>
#include <cstdint>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

#if _LIBCPP_STD_VER >= 26

#if __has_builtin(__builtin_pointers_related)

#ifndef __cpp_lib_is_pointer_between
#define __cpp_lib_is_pointer_between 202606L
#endif

_LIBCPP_HIDE_FROM_ABI constexpr bool is_pointer_between(const void * __ptr, const void * __first, const void * __last) {
  if consteval {
    _LIBCPP_ASSERT_UNCATEGORIZED(__builtin_pointers_related(__first, __last), "Pointers __first and __last must be pointing to same top-level object.");
    if (!__builtin_pointers_related(__first, __ptr)) {
      return false;
    }
  }
  return (__first <= __ptr) && (__ptr < __last);
}
#endif

#endif // _LIBCPP_STD_VER >= 26

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___MEMORY_IS_POINTER_BETWEEN_H
