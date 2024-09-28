// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___TAG_POINTER_H
#define _LIBCPP___TAG_POINTER_H

#include <__config>
#include <__type_traits/is_trivially_copyable.h>
#include <__assert>
#include <__fwd/tuple.h>
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
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr auto tag_pointer(_T * _ptr, uintptr_t _value) noexcept -> tagged_pointer<_T, _Mask> {
#if __has_builtin(__builtin_pointer_tag)
  auto _result = tagged_pointer<_T, _Mask>{static_cast<_T*>(__builtin_pointer_tag(_ptr, _value, _Mask))};
#else
  // non-constexpr variant
  auto _result = tagged_pointer<_T, _Mask>{reinterpret_cast<T*>((reinterpret_cast<uintptr_t>(_ptr) & ~_Mask) | (_value & _Mask))};
#endif
  _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(_result.value() == _value, "value can't be recovered with provided mask");
  _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(_result.pointer() == _ptr, "pointer can't be recovered with provided mask");
  return _result;
}

template <uintptr_t _Mask, class _T> 
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr auto tag_pointer(_T * _ptr, uintptr_t _value) noexcept -> tagged_pointer<_T, _Mask> {
  return tag_pointer<_T, _Mask>(_ptr, _value);
}

template <class _T, uintptr_t _Mask = pointer_tag_mask<alignof(_T)>> 
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr auto unsafe_tagged_pointer_cast(_T * _ptr) noexcept -> tagged_pointer<_T, _Mask> {
  return tagged_pointer<_T, _Mask>{_ptr};
}

template <class _T, uintptr_t _Mask = pointer_tag_mask<alignof(_T)>> 
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr _T * unsafe_pointer_cast(tagged_pointer<_T, _Mask> _ptr) noexcept {
  return _ptr._ptr;
}

template <class _T, uintptr_t _Mask> 
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr _T * untag_pointer(tagged_pointer<_T, _Mask> _ptr) noexcept {
#if __has_builtin(__builtin_pointer_untag)
    return static_cast<_T*>(__builtin_pointer_untag(_ptr._ptr, _Mask));
#else
    // non-constexpr variant
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(_ptr._ptr) & ~_Mask);
#endif
}

template <class _T, uintptr_t _Mask> 
[[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr uintptr_t obtain_tag_value(tagged_pointer<_T, _Mask> _ptr) noexcept {
#if __has_builtin(__builtin_pointer_tag_value)
  return __builtin_pointer_tag_value(_ptr._ptr, _Mask);
#else
  // non-constexpr variant
  return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(_ptr._ptr) & _Mask);
#endif
}

template <class _T, uintptr_t _Mask> class tagged_pointer {
  _T * _ptr{nullptr};
  
  _LIBCPP_HIDE_FROM_ABI explicit constexpr tagged_pointer(_T * _p) noexcept: _ptr{_p} { }
public:
  tagged_pointer() = default;
  tagged_pointer(const tagged_pointer &) = default;
  tagged_pointer(tagged_pointer &&) = default;
  ~tagged_pointer() = default;
  
  tagged_pointer & operator=(const tagged_pointer &) = default;
  tagged_pointer & operator=(tagged_pointer &&) = default;
  
  constexpr friend auto tag_pointer<_T, _Mask>(_T * ptr, uintptr_t _value) noexcept -> tagged_pointer<_T, _Mask>;
  
  constexpr friend auto unsafe_tagged_pointer_cast<_T, _Mask>(_T * ptr) noexcept -> tagged_pointer<_T, _Mask>;
  constexpr friend _T * unsafe_pointer_cast(tagged_pointer _ptr) noexcept;
  
  constexpr friend _T * untag_pointer<>(tagged_pointer _ptr) noexcept;
  constexpr friend uintptr_t obtain_tag_value<>(tagged_pointer _ptr) noexcept;
  
  template <std::size_t I> [[nodiscard]] _LIBCPP_HIDE_FROM_ABI constexpr auto get() const noexcept {
    static_assert(I <= 1, "std::tagged_pointer has only 2 elements!");
    if constexpr (I == 0) {
      return untag_pointer(*this);
    } else {
      return obtain_tag_value(*this);
    }
  }
};

template <typename T, uintptr_t Mask> struct tuple_size<tagged_pointer<T, Mask>>: std::integral_constant<std::size_t, 2> { };

template<typename T, uintptr_t Mask, size_t I>
struct tuple_element<I, tagged_pointer<T, Mask>>
{
    static_assert(I < 2, "std::tagged_pointer has only 2 elements!");
};

template<typename T, uintptr_t Mask>
struct tuple_element<0, tagged_pointer<T, Mask>>
{
    using type = T *;
};
 
template<typename T,uintptr_t Mask>
struct tuple_element<1, tagged_pointer<T, Mask>>
{
    using type = uintptr_t;
};

#endif // _LIBCPP_STD_VER >= 26

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___tag_pointer_H
