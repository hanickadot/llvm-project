// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___POINTER_TAG_H
#define _LIBCPP___POINTER_TAG_H

#include <__config>
#include <__type_traits/is_trivially_copyable.h>
#include "has_single_bit.h"
#include "popcount.h"

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

#if _LIBCPP_STD_VER >= 26
  
template <unsigned _Alignment> requires (std::has_single_bit(_Alignment)) constexpr uintptr_t pointer_tag_mask = (_Alignment) - 1u;

constexpr uintptr_t mask_for_alignment_2 = std::pointer_tag_mask<2>;
constexpr uintptr_t mask_for_alignment_4 = std::pointer_tag_mask<4>;
constexpr uintptr_t mask_for_alignment_8 = std::pointer_tag_mask<8>;

template <class _T> 
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr _T * pointer_tag(_T * _ptr, uintptr_t _value, uintptr_t _mask = std::pointer_tag_mask<alignof(_T)>) noexcept {
#if __has_builtin(__builtin_pointer_tag)
  return static_cast<_T*>(__builtin_pointer_tag(_ptr, _value, _mask));
#else
  return reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(ptr) & ~mask) | (value & mask));
#endif
}

template <class _T> 
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr _T * pointer_untag(_T * _ptr, uintptr_t _mask = std::pointer_tag_mask<alignof(_T)>) noexcept {
#if __has_builtin(__builtin_pointer_untag)
  return static_cast<_T*>(__builtin_pointer_untag(_ptr, _mask));
#else
  return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) & ~mask);
#endif
}

template <class _T> 
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr uintptr_t pointer_tag_value(_T * _ptr, uintptr_t _mask = std::pointer_tag_mask<alignof(_T)>) noexcept {
#if __has_builtin(__builtin_pointer_tag_value)
  return __builtin_pointer_tag_value(_ptr, _mask);
#else
  return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(ptr) & mask);
#endif
}

#endif // _LIBCPP_STD_VER >= 26

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___POINTER_TAG_H
