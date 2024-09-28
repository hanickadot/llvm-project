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
#include "__bit/has_single_bit.h"
#include <tuple>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

#if _LIBCPP_STD_VER >= 26

template <std::unsigned_integral _T = unsigned> struct _pointer_bit_mask {
  _T _value;

  constexpr operator _T() const noexcept {
    return _value;
  }
};

template <unsigned _Alignment> requires (std::has_single_bit(_Alignment)) constexpr auto alignment_mask = _pointer_bit_mask{(_Alignment) - 1u};

template <typename _T> constexpr auto alignment_mask_of = alignment_mask<alignof(_T)>;

template <typename _PointeeT, std::unsigned_integral auto _Mask = alignment_mask_of<_PointeeT>._value, std::unsigned_integral _TagT = unsigned> class pointer_tag_pair {
public:
  using pointee_type = _PointeeT;
  using pointer_type = pointee_type *;
  using tag_type = _TagT;
  using mask_type = decltype(_Mask);
  
  static constexpr mask_type mask = _Mask;
private:
  static constexpr bool is_custom = false;
  
  pointer_type _ptr;
  
  _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE static constexpr pointer_type encode_pointer_tag(pointer_type _pointer, tag_type _tag) noexcept {
#if __has_builtin(__builtin_pointer_tag)
    return static_cast<pointer_type>(__builtin_pointer_tag(_pointer, _tag, mask));
#else
    return reinterpret_cast<pointer_type>((reinterpret_cast<uintptr_t>(_pointer) & ~static_cast<uintptr_t>(mask)) | (static_cast<mask_type>(_tag) & mask));
#endif
  }
  _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE static constexpr pointer_type decode_pointer(pointer_type _pointer) noexcept {
#if __has_builtin(__builtin_pointer_untag)
    return static_cast<pointer_type>(__builtin_pointer_untag(_pointer, mask));
#else
    static_assert(sizeof(uintptr_t) == sizeof(pointer_type));
    return reinterpret_cast<pointer_type>(reinterpret_cast<uintptr_t>(_pointer) & reinterpret_cast<uintptr_t>(~mask));
#endif    
  }
  _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE static constexpr tag_type decode_tag(pointer_type _pointer) noexcept {
#if __has_builtin(__builtin_pointer_untag)
    return static_cast<tag_type>(__builtin_pointer_tag_value(_pointer, mask));
#else
    static_assert(sizeof(uintptr_t) == sizeof(_pointer_t));
    return reinterpret_cast<_pointer_t>(reinterpret_cast<uintptr_t>(_pointer) & reinterpret_cast<uintptr_t>(~mask));
#endif    
  }
public:
  constexpr pointer_tag_pair(pointer_type _pointer, tag_type _tag) noexcept requires (!is_custom): _ptr{encode_pointer_tag(_pointer, _tag)} { }
  
  template <typename _T = pointee_type> friend constexpr _T * pointer_cast(pointer_tag_pair _pair) noexcept requires (!is_custom){
    return static_cast<_T *>(decode_pointer(_pair._ptr));
  }
  
  template <typename _T = tag_type> friend constexpr _T value_cast(pointer_tag_pair _pair) noexcept requires (!is_custom){
    return static_cast<_T>(decode_tag(_pair._ptr));
  }
  
  template <std::size_t I> friend constexpr decltype(auto) get(pointer_tag_pair _pair) noexcept {
    static_assert(I < 2);
    if constexpr (I == 0) {
      return pointer_cast(_pair);
    } else {
      return value_cast(_pair);
    }
  }
};

// "casting" into pointer_tag_pair
template <std::unsigned_integral auto _Mask, typename _PointeeT> constexpr auto tag_pointer(_PointeeT * _ptr, std::unsigned_integral auto _tag = 0u) noexcept {
  return pointer_tag_pair<_PointeeT, _Mask, decltype(_tag)>(_ptr, _tag);
}

template <typename _PointeeT, std::unsigned_integral auto _Mask = alignment_mask_of<_PointeeT>._value, std::unsigned_integral _TagT = unsigned> constexpr auto tag_pointer(_PointeeT * _ptr, _TagT _tag = 0u) noexcept {
  return pointer_tag_pair<_PointeeT, _Mask, _TagT>(_ptr, _tag);
}

// tuple protocol support
template <typename _PointeeT, std::unsigned_integral auto _Mask, std::unsigned_integral _TagT>
struct tuple_size<pointer_tag_pair<_PointeeT, _Mask, _TagT>>: std::integral_constant<std::size_t, 2> {};

template <std::size_t I, typename _PointeeT, std::unsigned_integral auto _Mask, std::unsigned_integral _TagT>
struct tuple_element<
    I, pointer_tag_pair<_PointeeT, _Mask, _TagT>> {
  using _pair_type = pointer_tag_pair<_PointeeT, _Mask, _TagT>;
  using type = std::conditional_t<I == 0, typename _pair_type::pointer_type, typename _pair_type::tag_type>;
};

#endif // _LIBCPP_STD_VER >= 26

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___tag_pointer_H
