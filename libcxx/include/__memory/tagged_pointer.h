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

template <std::unsigned_integral auto _Mask> struct fixed_pointer_mask_t {
  using mask_type = std::remove_cvref_t<decltype(_Mask)>;
  
  static constexpr unsigned minimum_alignment = 1;
  
  consteval operator mask_type() const noexcept {
    return _Mask;
  }
};

template <unsigned _Alignment> struct static_alignment_mask_t {
  static_assert(std::has_single_bit(_Alignment) == 1, "Alignment must be power of 2");
  using mask_type = unsigned;

  static constexpr unsigned minimum_alignment = 1024;

  consteval operator mask_type() const noexcept {
    return _Alignment - 1u;
  }
};

// for future
struct dynamic_bit_mask { };

template <std::unsigned_integral auto _Mask> constexpr auto bit_mask = fixed_pointer_mask_t<_Mask>();

template <unsigned _Alignment> requires (std::has_single_bit(_Alignment)) constexpr auto alignment_mask = static_alignment_mask_t<_Alignment>{};

template <typename _T> constexpr auto type_alignment_mask = alignment_mask<alignof(_T)>;

// type tag to access special constructor
struct native_tagged_pointer_t { explicit constexpr native_tagged_pointer_t(int) noexcept { } };

constexpr auto native_tagged_pointer = native_tagged_pointer_t{0};

template <typename T> concept static_pointer_mask = requires() {
  requires std::unsigned_integral<typename T::mask_type>;
  requires std::convertible_to<T, typename T::mask_type>;
};

template <typename _PointeeT, std::unsigned_integral _TagT, auto _MaskV> class tagged_pointer;



static_assert(static_pointer_mask<static_alignment_mask_t<8u>>);
static_assert(static_pointer_mask<fixed_pointer_mask_t<0b111u>>);

template <typename _PointeeT, std::unsigned_integral _TagT, static_pointer_mask auto _MaskV> class tagged_pointer<_PointeeT, _TagT, _MaskV> {
public:
  using pointee_type = _PointeeT;
  using pointer_type = pointee_type *;
  using tag_type = _TagT;
  using mask_type = decltype(_MaskV);
  
  static constexpr auto mask = _MaskV; 
  
private:
  pointer_type _ptr{nullptr};
  
  // make other tagged_pointer types friendly so they can call from native constructor
  template <typename, std::unsigned_integral, auto> friend class tagged_pointer;
   
  _LIBCPP_ALWAYS_INLINE static constexpr pointer_type encode_pointer(pointer_type _pointer, tag_type _tag) noexcept {
#if __has_builtin(__builtin_pointer_tag)
    return static_cast<pointer_type>(__builtin_pointer_tag(_pointer, _tag, mask));
#else
    return reinterpret_cast<pointer_type>((reinterpret_cast<uintptr_t>(_pointer) & ~static_cast<uintptr_t>(mask)) | (reinterpret_cast<uintptr_t>(mask) & static_cast<uintptr_t>(_tag)));
#endif
  }
  
public:
  tagged_pointer() = default;
  
  _LIBCPP_ALWAYS_INLINE constexpr tagged_pointer(native_tagged_pointer_t, pointer_type _pointer) noexcept: _ptr{_pointer} { }
  
  // create pointer_tag by storing value into ptr
  _LIBCPP_ALWAYS_INLINE explicit constexpr tagged_pointer(pointer_type _pointer, tag_type _tag = 0u) noexcept: _ptr{encode_pointer(_pointer, _tag)} {
#if __has_builtin(__builtin_is_aligned)
    //_LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(__builtin_is_aligned(_pointer, mask.minimum_alignment), "provided pointer must have minimal alignment defined by mask");
#endif
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(this->pointer() == _pointer, "tagged pointer's mask must not hide any important pointer bits");
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(this->tag() == _tag, "tagged pointer's mask must not hide any tag bits");
  }
  
  _LIBCPP_ALWAYS_INLINE explicit constexpr tagged_pointer(pointer_type _pointer, tag_type _tag, mask_type) noexcept: tagged_pointer(_pointer, _tag) {
    // third argument is here only for deduction
  }
  
  _LIBCPP_ALWAYS_INLINE explicit constexpr tagged_pointer(pointer_type _pointer, mask_type) noexcept: tagged_pointer(_pointer) {
    // third argument is here only for deduction
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
  template <typename _P = pointee_type> _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE constexpr _P * pointer() const noexcept {
#if __has_builtin(__builtin_pointer_untag)
    return static_cast<_P *>(__builtin_pointer_untag(_ptr, mask));
#else
    return reinterpret_cast<_P *>(reinterpret_cast<uintptr_t>(_ptr) & ~static_cast<uintptr_t>(mask));
#endif
  }
  
  template <std::unsigned_integral _Tag = tag_type> _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE constexpr _Tag tag() const noexcept {
#if __has_builtin(__builtin_pointer_tag_value)
    return static_cast<_Tag>(__builtin_pointer_tag_value(_ptr, mask));
#else
    return static_cast<_Tag>(reinterpret_cast<uintptr_t>(_ptr) & static_cast<uintptr_t>(mask));
#endif
  }
  
  constexpr auto copy_and_tag(tag_type _new_tag) const noexcept -> tagged_pointer {
    return tagged_pointer{pointer(), _new_tag};
  }
  
  // tuple access support for structured bindings
  template <std::size_t I> _LIBCPP_NODISCARD _LIBCPP_ALWAYS_INLINE friend constexpr decltype(auto) get(tagged_pointer _pair) noexcept {
    static_assert(I < 3);
    if constexpr (I == 0) {
      return _pair.pointer();
    } else {
      return _pair.tag();
    }
  }
  
  // cast to change mask
  template <static_pointer_mask auto _NewMask> _LIBCPP_NODISCARD friend constexpr auto mask_cast(tagged_pointer _pair) noexcept {
    const auto output = tagged_pointer<pointee_type, tag_type, _NewMask>(native_tagged_pointer, _pair._ptr);
    
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.pointer(_new_mask) == pointer(_previous_mask), "pointer value must be untouched after convert_to_mask");
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value(_new_mask) == value(_previous_mask), "pointer value must be untouched after convert_to_mask");
    return output;
  }
  
  // cast to change tag type
  template <typename _OtherTagT> _LIBCPP_NODISCARD friend constexpr auto tag_cast(tagged_pointer _pair) noexcept {
    return tagged_pointer<pointee_type, _OtherTagT, mask>(native_tagged_pointer, _pair._ptr);
  }
  
  // const cast
  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend constexpr auto const_pointer_cast(tagged_pointer _pair) noexcept {
    return tagged_pointer<_OtherPointeeT, tag_type, mask>(native_tagged_pointer, const_cast<_OtherPointeeT*>(_pair._ptr));
  }
  
  // static cast
  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend constexpr auto static_pointer_cast(tagged_pointer _pair) noexcept {
    auto * result_ptr = static_cast<_OtherPointeeT *>(_pair.pointer());
    const auto output = tagged_pointer<_OtherPointeeT, tag_type, mask>(result_ptr, _pair.tag());

    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value() == result_ptr, "pointer value must be same as result of static_cast");
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value() == value(), "tag value must be untouched after convert_to_mask");
    return output;
  }
  
  // dynamic cast
  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend constexpr auto dynamic_pointer_cast(tagged_pointer _pair) noexcept {
    auto * result_ptr = dynamic_cast<_OtherPointeeT *>(_pair.pointer());
    const auto output = tagged_pointer<_OtherPointeeT, tag_type, mask>(result_ptr, _pair.tag());

    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value() == result_ptr, "pointer value must be same as result of dynamic_cast");
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value() == value(), "tag value must be untouched after convert_to_mask");
    return output;
  }
  
  // reinterpret cast
  template <typename _OtherPointeeT> _LIBCPP_NODISCARD friend constexpr auto reinterpret_pointer_cast(tagged_pointer _pair) noexcept {
    auto * result_ptr = reinterpret_cast<_OtherPointeeT *>(_pair.pointer());
    const auto output = tagged_pointer<_OtherPointeeT, tag_type, mask>(result_ptr, _pair.tag());

    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value() == result_ptr, "pointer value must be same as result of reinterpret_cast");
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(output.value() == value(), "tag value must be untouched after convert_to_mask");
    return output;
  }
  
  // operator access for pointers
  constexpr decltype(auto) operator*() const noexcept {
    return *pointer();
  }
  
  constexpr decltype(auto) operator->() const noexcept {
    return pointer();
  }
  
  // comparing objects
  friend constexpr auto operator<=>(tagged_pointer _lhs, tagged_pointer _rhs) noexcept {
    // ordering by pointer address AND then with tag (which can change upper bits)
    return std::tuple(_lhs.pointer(), _lhs.tag()) <=> std::tuple(_rhs.pointer(), _rhs.tag());
  }
  
  friend constexpr bool operator==(tagged_pointer _lhs, tagged_pointer _rhs) noexcept {
    return _lhs._ptr == _rhs._ptr;
  }
  
  friend constexpr auto operator<=>(tagged_pointer _lhs, pointer_type _rhs) noexcept {
    // ordering by pointer address AND then with tag (which can change upper bits)
    return _lhs.pointer() <=> _rhs;
  }
  
  friend constexpr bool operator==(tagged_pointer _lhs, pointer_type _rhs) noexcept {
    return _lhs.pointer() == _rhs;
  }
  
  friend constexpr bool operator==(tagged_pointer _lhs, nullptr_t _rhs) noexcept {
    return _lhs.pointer() == nullptr;
  }
  
  explicit constexpr operator bool() const noexcept {
    return *this == nullptr;
  }
};

// CTAD
template <typename _PointerT> tagged_pointer(_PointerT *) -> tagged_pointer<_PointerT, uintptr_t, type_alignment_mask<_PointerT>>;

template <typename _PointerT, typename _TagT> tagged_pointer(_PointerT *, _TagT) -> tagged_pointer<_PointerT, _TagT, type_alignment_mask<_PointerT>>;

template <typename _PointerT, typename _TagT, static_pointer_mask _MaskT> tagged_pointer(_PointerT *, _TagT, _MaskT) -> tagged_pointer<_PointerT, _TagT, _MaskT{}>;

template <typename _PointerT, static_pointer_mask _MaskT> tagged_pointer(_PointerT *, _MaskT) -> tagged_pointer<_PointerT, uintptr_t, _MaskT{}>;

// unsafe construct tagged_pointer from native_pointer

template <typename _PointerT, std::unsigned_integral _TagT = unsigned, static_pointer_mask _MaskT>
constexpr auto convert_native_tagged_pointer(_PointerT * _pointer, _MaskT) noexcept {
  return tagged_pointer<_PointerT, _TagT, _MaskT{}>{native_tagged_pointer, _pointer};
}

// tuple protocol support
template <typename _PointeeT, std::unsigned_integral _TagT, auto _MaskV>
struct tuple_size<tagged_pointer<_PointeeT, _TagT, _MaskV>>: std::integral_constant<std::size_t, 2> {};

template <std::size_t I, typename _PointeeT, std::unsigned_integral _TagT, auto _MaskV>
struct tuple_element<
    I, tagged_pointer<_PointeeT, _TagT, _MaskV>> {
  using _pair_type = tagged_pointer<_PointeeT, _TagT, _MaskV>;
  using type = std::conditional_t<I == 0, typename _pair_type::pointer_type, typename _pair_type::tag_type>;
};


// hashing
template <typename _PointeeT, std::unsigned_integral _TagT, auto _MaskV>
struct _LIBCPP_TEMPLATE_VIS hash<tagged_pointer<_PointeeT, _TagT, _MaskV>>: hash<std::pair<_PointeeT, _TagT>> {
  using _parent_pair_hasher = hash<std::pair<_PointeeT, _TagT>>;
  
  std::size_t operator()(tagged_pointer<_PointeeT, _TagT, _MaskV> _pair) const noexcept {
    return this->_parent_pair_hasher::operator()({_pair.pointer(), _pair.tag()});
  }
};

// pointer traits
template <typename _PointeeT, std::unsigned_integral _TagT, auto _MaskV>
struct _LIBCPP_TEMPLATE_VIS pointer_traits<tagged_pointer<_PointeeT, _TagT, _MaskV>> {
  typedef _PointeeT* pointer;
  typedef _PointeeT element_type;
  typedef ptrdiff_t difference_type;

  // what to do with this?
  template <class _Up>
  using rebind = _Up*;

public:
  _LIBCPP_HIDE_FROM_ABI constexpr static pointer
  pointer_to(tagged_pointer<_PointeeT, _TagT, _MaskV> _pair) _NOEXCEPT {
    return _pair.pointer();
  }
};

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP_STD_VER >= 26

#endif // _LIBCPP___TAGGED_POINTER_H
