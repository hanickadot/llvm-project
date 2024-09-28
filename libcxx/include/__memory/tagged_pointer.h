// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___TAGGED_POINTER_H
#define _LIBCPP___TAGGED_POINTER_H

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCPP_STD_VER >= 26
  
#include <__config>
#include <__type_traits/is_trivially_copyable.h>
#include <__assert>
#include "__bit/has_single_bit.h"
#include "pointer_traits.h"
#include <tuple>
#include <compare>

_LIBCPP_BEGIN_NAMESPACE_STD

template <uintptr_t _MaskT> struct _pointer_bit_mask_t {
  constexpr operator uintptr_t() const noexcept {
    return static_cast<uintptr_t>(_MaskT);
  }
  static constexpr uintptr_t dynamic = _MaskT;
};

template <uintptr_t _MaskT> constexpr auto pointer_bit_mask = _pointer_bit_mask_t<_MaskT>{};

template <unsigned _Alignment> requires (std::has_single_bit(_Alignment)) constexpr auto alignment_mask = pointer_bit_mask<(_Alignment - 1u)>;

template <typename _T> constexpr auto type_alignment_mask = alignment_mask<alignof(_T)>;

template <auto _Alignment> constexpr auto dynamic = static_cast<uintptr_t>(_Alignment);

struct _native_tagged_pointer_t { explicit constexpr _native_tagged_pointer_t(int) noexcept { } };

constexpr auto _native_tagged_pointer = _native_tagged_pointer_t{0};

template <typename T> concept pointer_mask = std::convertible_to<T, uintptr_t>;
template <typename T> concept dynamic_mask = !std::is_empty_v<T>;
template <typename T> concept static_mask = std::is_empty_v<T>;

template <typename _PointeeT, std::unsigned_integral _TagT, pointer_mask _MaskT> class tagged_pointer {
public:
  using pointee_type = _PointeeT;
  using pointer_type = pointee_type *;
  using tag_type = _TagT;
  using mask_type = _MaskT;
  
  static constexpr bool has_static_mask = static_mask<mask_type>;
  static constexpr bool has_dynamic_mask = dynamic_mask<mask_type>;
  
private:
  pointer_type _ptr{nullptr};
  
  // make other tagged_pointer types friendly so they can call from native constructor
  template <typename, std::unsigned_integral, pointer_mask> friend class tagged_pointer;
  
  // verbose function to construct std::tagged_pointer from native tagged pointer type
  template <typename _OtherTagT, typename _OtherPointerT, pointer_mask _OtherMaskT> friend constexpr auto convert_to_tagged_pointer(_OtherPointerT * _pointer, _OtherMaskT) -> tagged_pointer<_OtherPointerT, _OtherTagT, _OtherMaskT>;
  
  // hidden constructor from pointer type (usable in conversion, and from-native construction)
  _LIBCPP_ALWAYS_INLINE constexpr tagged_pointer(_native_tagged_pointer_t, pointer_type _pointer) noexcept: _ptr{_pointer} { }
  
public:
  tagged_pointer() = default;
  
  // create pointer_tag by storing value into ptr
  _LIBCPP_ALWAYS_INLINE explicit constexpr tagged_pointer(pointer_type _pointer, tag_type _tag = 0u, mask_type _mask = type_alignment_mask<_PointeeT>) noexcept:
#if __has_builtin(__builtin_pointer_tag)
    _ptr{static_cast<pointer_type>(__builtin_pointer_tag(_pointer, _tag, _mask))}
#else
    _ptr{reinterpret_cast<pointer_type>((reinterpret_cast<uintptr_t>(_pointer) & ~static_cast<uintptr_t>(_mask)) | (reinterpret_cast<uintptr_t>(_mask) & static_cast<uintptr_t>(_tag)))}
#endif
  {
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(this->pointer(_mask) == _pointer, "provided mask must not hide any important pointer bits");
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(this->tag(_mask) == _tag, "provided mask must not hide any tag bits");
  }
  
  tagged_pointer(const tagged_pointer &) = default;
  tagged_pointer(tagged_pointer &&) = default;
  ~tagged_pointer() = default;
  
  tagged_pointer & operator=(const tagged_pointer &) = default;
  tagged_pointer & operator=(tagged_pointer &&) = default;
  
  friend constexpr void swap(tagged_pointer & lhs, tagged_pointer & rhs) noexcept {
    std::swap(lhs._ptr, rhs._ptr);
  }
  
  // function so this type can interact with other pointer tagging facilities
  _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE constexpr pointer_type native_pointer() const noexcept {
    return _ptr;
  }
  
  // user must provide mask as it's not known at compile-time
  template <typename _P = pointee_type> _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE constexpr _P * pointer(mask_type mask) const noexcept {
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT((dynamic_mask<mask_type>) || (mask == mask_type{}), "provided mask must be dynamic or must always be converted to same value");
#if __has_builtin(__builtin_pointer_untag)
    return static_cast<_P *>(__builtin_pointer_untag(_ptr, mask));
#else
     return reinterpret_cast<_P *>(reinterpret_cast<uintptr_t>(_ptr) & ~static_cast<uintptr_t>(_mask));
#endif
  }
  
  template <std::unsigned_integral _Tag = tag_type> _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE constexpr _Tag tag(mask_type mask) const noexcept {
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT((dynamic_mask<mask_type>) || (mask == mask_type{}), "provided mask must be dynamic or must always be converted to same value");
#if __has_builtin(__builtin_pointer_tag_value)
    return static_cast<_Tag>(__builtin_pointer_tag_value(_ptr, mask));
#else
    return static_cast<_Tag>(reinterpret_cast<uintptr_t>(_ptr) & static_cast<uintptr_t>(_mask));
#endif
  }
  
  // with defaulted mask which is usable only if it's compile-time known
  template <typename _P = pointee_type> _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE constexpr _P * pointer() const noexcept requires (has_static_mask) {
    return pointer({});
  }

  template <std::unsigned_integral _Tag = tag_type> _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE constexpr _Tag tag() const noexcept requires (has_static_mask) {
    return tag<_Tag>({});
  }
  
  // tuple access support for structured bindings
  template <std::size_t I> _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE friend constexpr decltype(auto) get(tagged_pointer _pair) noexcept requires (has_static_mask) {
    static_assert(I < 3);
    if constexpr (I == 0) {
      return _pair.pointer();
    } else {
      return _pair.tag();
    }
  }
  
  // we need to be able to change mask
  _LIBCPP_NODISCARD friend constexpr auto mask_cast(pointer_mask auto _new_mask, mask_type _previous_mask, tagged_pointer _pair) noexcept {
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT((dynamic_mask<mask_type>) || (_previous_mask == mask_type{}), "provided previous mask must be dynamic or must always be converted to same value");
    
    const auto output = tagged_pointer<pointee_type, tag_type, decltype(_new_mask)>(_native_tagged_pointer, _pair._ptr);
    
    (void)(_previous_mask); // with disabled asserts mask is not touched
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.pointer(_new_mask) == pointer(_previous_mask), "pointer value must be untouched after convert_to_mask");
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value(_new_mask) == value(_previous_mask), "pointer value must be untouched after convert_to_mask");
    return output;
  }
  
  _LIBCPP_NODISCARD friend constexpr auto mask_cast(pointer_mask auto _new_mask, tagged_pointer _pair) noexcept requires (has_static_mask) {
    return mask_cast(_new_mask, mask_type{}, _pair);
  }
  
  // TAG TYPE CAST
  template <typename _OtherTagT> _LIBCPP_NODISCARD friend constexpr auto tag_cast(tagged_pointer _pair) noexcept {
   return tagged_pointer<pointee_type, _OtherTagT, mask_type>(_native_tagged_pointer, _pair._ptr);
  }
  
  // CONST_CAST (doesn't change pointer, only CV qualifier)
  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend constexpr auto const_pointer_cast(tagged_pointer _pair) noexcept {
    return tagged_pointer<_OtherPointeeT, tag_type, mask_type>(_native_tagged_pointer, const_cast<_OtherPointeeT*>(_pair._ptr));
  }
  
  // static_cast
  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend constexpr auto static_pointer_cast(mask_type _mask, tagged_pointer _pair) noexcept {
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT((dynamic_mask<mask_type>) || (_mask == mask_type{}), "provided mask must be dynamic or must always be converted to same value");

    const auto output = tagged_pointer<_OtherPointeeT, tag_type, mask_type>(static_cast<_OtherPointeeT *>(_pair.pointer(_mask)), _pair.tag(_mask), _mask);

    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value(_new_mask) == value(_previous_mask), "pointer value must be untouched after convert_to_mask");
    return output;
  }

  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend constexpr auto static_pointer_cast(tagged_pointer _pair) noexcept requires (has_static_mask) {
    return static_pointer_cast<_OtherPointeeT>({}, _pair);
  }
  
  // dynamic_cast
  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend constexpr auto dynamic_pointer_cast(mask_type _mask, tagged_pointer _pair) noexcept {
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT((dynamic_mask<mask_type>) || (_mask == mask_type{}), "provided mask must be dynamic or must always be converted to same value");

    const auto output = tagged_pointer<_OtherPointeeT, tag_type, mask_type>(dynamic_cast<_OtherPointeeT *>(_pair.pointer(_mask)), _pair.tag(_mask), _mask);

    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value(_new_mask) == value(_previous_mask), "pointer value must be untouched after convert_to_mask");
    return output;
  }

  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend constexpr auto dynamic_pointer_cast(tagged_pointer _pair) noexcept requires (has_static_mask) {
    return dynamic_pointer_cast<_OtherPointeeT>({}, _pair);
  }
  
  // reinterpret_cast
  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend auto reinterpret_pointer_cast(mask_type _mask, tagged_pointer _pair) noexcept {
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT((dynamic_mask<mask_type>) || (_mask == mask_type{}), "provided mask must be dynamic or must always be converted to same value");

    const auto output = tagged_pointer<_OtherPointeeT, tag_type, mask_type>(reinterpret_cast<_OtherPointeeT *>(_pair.pointer(_mask)), _pair.tag(_mask), _mask);

    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value(_new_mask) == value(_previous_mask), "pointer value must be untouched after convert_to_mask");
    return output;
  }

  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend auto reinterpret_pointer_cast(tagged_pointer _pair) noexcept requires (has_static_mask) {
    return reinterpret_pointer_cast<_OtherPointeeT>({}, _pair);
  }
  
  // comparing objects
  friend constexpr auto operator<=>(tagged_pointer _lhs, tagged_pointer _rhs) noexcept requires (has_static_mask) {
    // ordering by pointer address AND then with tag (which can change upper bits)
    return std::tuple(_lhs.pointer(), _lhs.tag()) <=> std::tuple(_rhs.pointer(), _rhs.tag());
  }
  
  friend constexpr bool operator==(tagged_pointer _lhs, tagged_pointer _rhs) noexcept requires (has_static_mask) {
    return _lhs._ptr == _rhs._ptr;
  }
  
  friend constexpr auto operator<=>(tagged_pointer _lhs, pointer_type _rhs) noexcept requires (has_static_mask) {
    // ordering by pointer address AND then with tag (which can change upper bits)
    return _lhs.pointer() <=> _rhs;
  }
  
  friend constexpr bool operator==(tagged_pointer _lhs, pointer_type _rhs) noexcept requires (has_static_mask) {
    return _lhs.pointer() == _rhs;
  }
  
  friend constexpr bool operator==(tagged_pointer _lhs, nullptr_t _rhs) noexcept requires (has_static_mask) {
    return _lhs.pointer() == nullptr;
  }
};

// unsafe construct tagged_pointer from native_pointer
// different order as _PointerT will be deduced, but _Tag 
template <typename _TagT = uintptr_t, typename _PointerT, pointer_mask _MaskT> constexpr auto convert_to_tagged_pointer(_PointerT * _pointer, _MaskT) -> tagged_pointer<_PointerT, _TagT, _MaskT> {
  return {_native_tagged_pointer, _pointer};
}

template <typename _TagT = uintptr_t, typename _PointerT> constexpr auto convert_to_tagged_pointer_with_unknown_mask(_PointerT * _pointer) -> tagged_pointer<_PointerT, _TagT, uintptr_t> {
  return convert_to_tagged_pointer<_TagT, _PointerT, uintptr_t>(_pointer, 0u);
}

// CTAD: everything is specified
template <typename _PointeeT, std::unsigned_integral _TagT, pointer_mask _MaskT> tagged_pointer(_PointeeT *, _TagT, _MaskT) -> tagged_pointer<_PointeeT, _TagT, _MaskT>;

// CTAD: pointer + tag provided (default mask)
template <typename _PointeeT, std::unsigned_integral _TagT> tagged_pointer(_PointeeT * _ptr, _TagT _tag) -> tagged_pointer<_PointeeT, _TagT, decltype(type_alignment_mask<_PointeeT>)>;

// CTAD: make pointer into tagged pointer
template <typename _PointeeT> tagged_pointer(_PointeeT * _ptr) -> tagged_pointer<_PointeeT, unsigned, decltype(type_alignment_mask<_PointeeT>)>;

// tuple protocol support
template <typename _PointeeT, std::unsigned_integral _TagT, pointer_mask _MaskT>
struct tuple_size<tagged_pointer<_PointeeT, _TagT, _MaskT>>: std::integral_constant<std::size_t, 2> {};

template <std::size_t I, typename _PointeeT, pointer_mask _MaskT, std::unsigned_integral _TagT>
struct tuple_element<
    I, tagged_pointer<_PointeeT, _TagT, _MaskT>> {
  using _pair_type = tagged_pointer<_PointeeT, _TagT, _MaskT>;
  using type = std::conditional_t<I == 0, typename _pair_type::pointer_type, typename _pair_type::tag_type>;
};

// hashing
template <typename _PointeeT, std::unsigned_integral _TagT, pointer_mask _MaskT> 
requires (static_mask<_MaskT>)
struct _LIBCPP_TEMPLATE_VIS hash<tagged_pointer<_PointeeT, _TagT, _MaskT>>: hash<std::pair<_PointeeT, _TagT>> {
  using _parent_pair_hasher = hash<std::pair<_PointeeT, _TagT>>;
  
  std::size_t operator()(tagged_pointer<_PointeeT, _TagT, _MaskT> _pair) const noexcept {
    return this->_parent_pair_hasher::operator()({_pair.pointer(), _pair.tag()});
  }
};

// pointer traits
template <typename _PointeeT, std::unsigned_integral _TagT, pointer_mask _MaskT>
struct _LIBCPP_TEMPLATE_VIS pointer_traits<tagged_pointer<_PointeeT, _TagT, _MaskT>> {
  typedef _PointeeT* pointer;
  typedef _PointeeT element_type;
  typedef ptrdiff_t difference_type;

  // what to do with this?
#ifndef _LIBCPP_CXX03_LANG
  template <class _Up>
  using rebind = _Up*;
#else
  template <class _Up>
  struct rebind {
    typedef _Up* other;
  };
#endif

public:
  _LIBCPP_HIDE_FROM_ABI constexpr static pointer
  pointer_to(tagged_pointer<_PointeeT, _TagT, _MaskT> _pair) _NOEXCEPT requires (static_mask<_MaskT>) {
    return _pair.pointer();
  }
};

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP_STD_VER >= 26

#endif // _LIBCPP___TAGGED_POINTER_H
