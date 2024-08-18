// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___tag_pointer_H
#define _LIBCPP___tag_pointer_H

#include <__config>
#include <__type_traits/is_trivially_copyable.h>
#include <__assert>
#include "has_single_bit.h"
#include "popcount.h"

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

#if _LIBCPP_STD_VER >= 26
  
template <unsigned _Alignment> requires (std::has_single_bit(_Alignment)) constexpr uintptr_t pointer_tag_mask = (_Alignment) - 1u;

template <class _T, uintptr_t _Mask = pointer_tag_mask<alignof(_T)>> class tagged_pointer;

template <class _T, uintptr_t _Mask = pointer_tag_mask<alignof(_T)>> 
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr auto tag_pointer(_T * _ptr, uintptr_t _value) noexcept -> tagged_pointer<_T, _Mask>;

template <typename _T, uintptr_t _Mask> [[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr _T * untag_pointer(tagged_pointer<_T, _Mask> _ptr) noexcept;

template <typename _T, uintptr_t _Mask> [[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr uintptr_t obtain_pointer_tag(tagged_pointer<_T, _Mask> _ptr) noexcept;

struct unsafe_tag_t { };

constexpr auto unsafe_tag = unsafe_tag_t{};

template <class _T, uintptr_t _Mask> class tagged_pointer {
  _T * _ptr{nullptr};
  
  explicit constexpr tagged_pointer(_T * _p) noexcept: _ptr{_p} { }
public:
  constexpr tagged_pointer(unsafe_tag_t, _T * _p) noexcept: _ptr{_p} { }
  
  tagged_pointer() = default;
  tagged_pointer(const tagged_pointer &) = default;
  tagged_pointer(tagged_pointer &&) = default;
  ~tagged_pointer() = default;
  
  tagged_pointer & operator=(const tagged_pointer &) = default;
  tagged_pointer & operator=(tagged_pointer &&) = default;
  
  constexpr _T * unsafe_pointer_value() const noexcept {
    return _ptr;
  }
  
  _LIBCPP_HIDE_FROM_ABI constexpr friend auto tag_pointer<_T, _Mask>(_T * ptr, uintptr_t _value) noexcept -> tagged_pointer<_T, _Mask>;
  
  _LIBCPP_HIDE_FROM_ABI constexpr friend _T * untag_pointer<_T, _Mask>(tagged_pointer<_T, _Mask> _ptr) noexcept;
  _LIBCPP_HIDE_FROM_ABI constexpr friend uintptr_t obtain_pointer_tag<_T, _Mask>(tagged_pointer<_T, _Mask> _ptr) noexcept;
};

template <typename _T, uintptr_t _Mask> [[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr _T * untag_pointer(tagged_pointer<_T, _Mask> _ptr) noexcept {
#if __has_builtin(__builtin_pointer_untag)
    return static_cast<_T*>(__builtin_pointer_untag(_ptr._ptr, _Mask));
#else
    // non-constexpr variant
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(_ptr._ptr) & ~_Mask);
#endif
}
template <typename _T, uintptr_t _Mask> [[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr uintptr_t obtain_pointer_tag(tagged_pointer<_T, _Mask> _ptr) noexcept {
#if __has_builtin(__builtin_pointer_tag_value)
  return __builtin_pointer_tag_value(_ptr._ptr, _Mask);
#else
  // non-constexpr variant
  return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(_ptr._ptr) & _Mask);
#endif
}

template <class _T, uintptr_t _Mask> 
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr auto tag_pointer(_T * _ptr, uintptr_t _value) noexcept -> tagged_pointer<_T, _Mask> {
#if __has_builtin(__builtin_pointer_tag)
  auto _result = tagged_pointer<_T, _Mask>{static_cast<_T*>(__builtin_pointer_tag(_ptr, _value, _Mask))};
#else
  // non-constexpr variant
  auto _result = tagged_pointer<_T, _Mask>{reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(_ptr) & ~_Mask) | (_value & _mask))};
#endif
  _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(std::obtain_pointer_tag(_result) == _value, "value can't be recovered with provided mask");
  _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(std::untag_pointer(_result) == _ptr, "pointer can't be recovered with provided mask");
  return _result;
}

#endif // _LIBCPP_STD_VER >= 26

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___tag_pointer_H
