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
#include <__type_traits/rank.h>
#include "pointer_traits.h"
#include <compare>

_LIBCPP_BEGIN_NAMESPACE_STD

template <typename T, typename Y> concept convertible_to_from = std::convertible_to<Y, T> && std::convertible_to<T, Y>;

template <typename T> concept pointer_mask = true;

template <typename T> concept pointer_tagging_schema = requires(T::dirty_pointer payload, T::clean_pointer clean, T::tag_type tag) {
  //requires convertible_to_from<typename T::tag_type, uintptr_t>;
  requires std::is_pointer_v<typename T::clean_pointer>;
  
  { T::tag_pointer(clean, tag) } noexcept -> std::same_as<typename T::dirty_pointer>;
  { T::pointer_mask(payload) } noexcept -> std::same_as<typename T::clean_pointer>;
  { T::value_mask(payload) } noexcept -> std::same_as<typename T::tag_type>;
};

template <typename T> concept pointer_tagging_schema_with_aliasing = pointer_tagging_schema<T> && requires(T::dirty_pointer payload) {
  { T::aliasing_pointer_mask(payload) } noexcept -> std::same_as<typename T::clean_pointer>;
};

template <typename T> struct voidify_pointer_t;

template <typename T> struct voidify_pointer_t<T *> {
  using result_type = void *;
};

template <typename T> struct voidify_pointer_t<const T *> {
  using result_type = const void *;
};

template <typename T> using voidify_pointer = voidify_pointer_t<T>::result_type;

template <typename PointerT, pointer_mask TagT = uintptr_t> struct dummy_tag {
  template <typename PointerT2> using rebind = dummy_tag<PointerT2, TagT>;
  using clean_pointer = PointerT;
  using dirty_pointer = voidify_pointer<PointerT>;
  using tag_type = uintptr_t;
  
  //static constexpr uintptr_t available_bits = 0;
  
  [[clang::always_inline]] static constexpr dirty_pointer tag_pointer(clean_pointer _ptr, tag_type _value) noexcept {
    return static_cast<dirty_pointer>(_ptr);
  }
  [[clang::always_inline]] static constexpr clean_pointer pointer_mask(dirty_pointer _ptr) noexcept {
    return static_cast<clean_pointer>(_ptr);
  }
  [[clang::always_inline]] static constexpr tag_type value_mask(dirty_pointer _ptr) noexcept {
    return 0;
  }
};

template <uintptr_t Mask, typename PointerT, pointer_mask TagT = uintptr_t> struct custom_bitmap_tag {
  template <typename PointerT2> using rebind = custom_bitmap_tag<Mask, PointerT2, TagT>;
  
  static constexpr auto available_bits = Mask;
 
  using clean_pointer = PointerT;
  using dirty_pointer = void *;
  using tag_type = TagT;
  
  [[clang::always_inline]] static constexpr dirty_pointer tag_pointer(clean_pointer _ptr, tag_type _value) noexcept {
    return static_cast<dirty_pointer>(__builtin_tag_pointer_mask_or((void *)(_ptr), static_cast<uintptr_t>(_value), available_bits));
  }
  [[clang::always_inline]] static constexpr clean_pointer pointer_mask(dirty_pointer _ptr) noexcept {
    return static_cast<clean_pointer>(__builtin_tag_pointer_mask((void *)_ptr, ~available_bits));
  }
  [[clang::always_inline]] static constexpr tag_type value_mask(dirty_pointer _ptr) noexcept {
    return static_cast<tag_type>(__builtin_tag_pointer_mask_as_int((void *)_ptr, available_bits));
  }
};

template <unsigned Shift, typename PointerT, pointer_mask TagT = uintptr_t> struct custom_low_bits_shift_tag {
  template <typename PointerT2> using rebind = custom_low_bits_shift_tag<Shift, PointerT2, TagT>;
  
  using clean_pointer = PointerT;
  using dirty_pointer = void *;
  using tag_type = TagT;
  
  static constexpr uintptr_t available_bits = (static_cast<uintptr_t>(1ull) << Shift) - 1ull;
  
  [[clang::always_inline]] static constexpr dirty_pointer tag_pointer(clean_pointer _ptr, tag_type _value) noexcept {
    return static_cast<dirty_pointer>(__builtin_tag_pointer_shift_or((void *)(_ptr), static_cast<uintptr_t>(_value), Shift));
  }
  [[clang::always_inline]] static constexpr clean_pointer pointer_mask(dirty_pointer _ptr) noexcept {
    return static_cast<clean_pointer>(__builtin_tag_pointer_unshift((void*)_ptr, Shift));
  }
  [[clang::always_inline]] static constexpr tag_type value_mask(dirty_pointer _ptr) noexcept {
    return static_cast<tag_type>(__builtin_tag_pointer_mask_as_int((void*)_ptr, available_bits));
  }
};

template <std::size_t Alignment> static constexpr uintptr_t mask_for_alignment = (static_cast<uintptr_t>(Alignment) - 1u);

// provided custom (over-)alignment
template <size_t Alignment, typename PointerT, pointer_mask TagT = uintptr_t> 
struct custom_alignment_low_bits_tag: custom_bitmap_tag<mask_for_alignment<Alignment>, PointerT, TagT> {
  using _super = custom_bitmap_tag<mask_for_alignment<Alignment>, PointerT, TagT>;
  
  template <typename PointerT2> using rebind = custom_alignment_low_bits_tag<Alignment, PointerT2, TagT>;
  
  static_assert(std::has_single_bit(Alignment), "alignment must be power of 2");
  static_assert(Alignment > 1, "you must use alignment bigger than 1 to use this schema");
  
  using clean_pointer = PointerT;
  using dirty_pointer = void *;
  using tag_type = TagT;
  
  [[clang::always_inline]] static constexpr dirty_pointer tag_pointer(clean_pointer _ptr, tag_type _value) noexcept {
#if __has_builtin(__builtin_is_aligned)
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(__builtin_is_aligned(_ptr, Alignment), "provided pointer must have minimal alignment");
#endif
    return _super::tag_pointer(_ptr, _value);
  }
  
};

//static_assert(custom_alignment_low_bits_tag<1, char *>::available_bits == 0b0u);
static_assert(custom_alignment_low_bits_tag<2, char *>::available_bits == 0b1u);
//static_assert(custom_alignment_low_bits_tag<3, char *>::available_bits == 0b1u); // this alignment doesn't make sense
static_assert(custom_alignment_low_bits_tag<4, char *>::available_bits == 0b11u);
static_assert(custom_alignment_low_bits_tag<8, char *>::available_bits == 0b111u);
static_assert(custom_alignment_low_bits_tag<16, char *>::available_bits == 0b1111u);
static_assert(custom_alignment_low_bits_tag<32, char *>::available_bits == 0b11111u);


// we just use default alignment
template <typename PointerT, pointer_mask TagT = uintptr_t> struct alignment_low_bits_tag: custom_alignment_low_bits_tag<alignof(decltype(*std::declval<PointerT>())), PointerT, TagT> {
  static constexpr unsigned alignment = alignof(decltype(*std::declval<PointerT>()));
  static_assert(alignment > 1, "this tagging schema needs pointer to a type with alignment > 1");
  // TODO check if the type has enough alignment
  template <typename PointerT2> using rebind = alignment_low_bits_tag<PointerT2, TagT>;
};

//static_assert(alignment_low_bits_tag<char *>::available_bits == 0b0u);
static_assert(alignment_low_bits_tag<short *>::available_bits == 0b1u);
static_assert(alignment_low_bits_tag<int *>::available_bits == 0b11u);
static_assert(alignment_low_bits_tag<long long *>::available_bits == 0b111u);


// different tagging schemas
template <typename PointerT, pointer_mask TagT = uint8_t> struct low_byte_tag: custom_low_bits_shift_tag<8, PointerT, TagT> {
  template <typename PointerT2> using rebind = low_byte_tag<PointerT2, TagT>;
};

static_assert(low_byte_tag<int *>::available_bits == 0xFFu);

// example of ARM native pointer tagging
static constexpr uintptr_t shift_for_upper_byte = (sizeof(void *) * 8u) - 8u;
static constexpr uintptr_t mask_for_upper_byte = uintptr_t(0xFF) << shift_for_upper_byte;

template <typename PointerT, pointer_mask TagT = uintptr_t> struct upper_byte_tag {
  using _super = custom_bitmap_tag<mask_for_upper_byte, PointerT, uintptr_t>;
  
  using clean_pointer = _super::clean_pointer;
  using dirty_pointer = _super::dirty_pointer;
  using tag_type = TagT;
  
  // note: no available_bits constexpr variable
  template <typename PointerT2> using rebind = upper_byte_tag<PointerT2, TagT>;
  
  [[clang::always_inline]] static constexpr dirty_pointer tag_pointer(clean_pointer _ptr, tag_type _value) noexcept {
    return _super::tag_pointer(_ptr, static_cast<uintptr_t>(_value));
  }
  [[clang::always_inline]] static constexpr clean_pointer aliasing_pointer_mask(dirty_pointer _ptr) noexcept {
    if consteval {
      return _super::pointer_mask(_ptr);
    }
    // we can just return our pointer, as we know our platform can safely dereference it
    return static_cast<clean_pointer>(_ptr); 
  }
  [[clang::always_inline]] static constexpr clean_pointer pointer_mask(dirty_pointer _ptr) noexcept {
    return _super::pointer_mask(_ptr);
  }
  [[clang::always_inline]] static constexpr tag_type value_mask(dirty_pointer _ptr) noexcept {
    return static_cast<tag_type>(_super::value_mask(_ptr));
  }
};

template <typename PointerT, pointer_mask TagT = uint8_t> struct upper_byte_shifted_tag: upper_byte_tag<PointerT, uintptr_t> {
  using _super = upper_byte_tag<PointerT, uintptr_t>;
  
  using clean_pointer = _super::clean_pointer;
  using dirty_pointer = _super::dirty_pointer;
  using tag_type = TagT;
  
  template <typename PointerT2> using rebind = upper_byte_shifted_tag<PointerT2, tag_type>;
  
  [[clang::always_inline]] static constexpr dirty_pointer tag_pointer(clean_pointer _ptr, tag_type _value) noexcept {
    return _super::tag_pointer(_ptr, static_cast<uintptr_t>(_value) << shift_for_upper_byte);
  }
  [[clang::always_inline]] static constexpr tag_type value_mask(dirty_pointer _ptr) noexcept {
    return static_cast<tag_type>(_super::value_mask(_ptr) >> shift_for_upper_byte);
  }
};










// forward declaration
template <pointer_tagging_schema _Scheme> class tagged_ptr;


// create tagged_ptr by specifying schema without any conversions
template <pointer_tagging_schema _Scheme> [[clang::always_inline]] constexpr auto tagged_pointer_cast(typename _Scheme::dirty_pointer _ptr) noexcept -> tagged_ptr<_Scheme> {
  return tagged_ptr<_Scheme>(_Scheme{}, _ptr);
}

template <typename T> struct is_tagged_ptr: std::false_type { };

template <pointer_tagging_schema _Schema> struct is_tagged_ptr<tagged_ptr<_Schema>>: std::true_type { };

template <typename T> concept a_tagged_ptr = is_tagged_ptr<T>::value;

template <a_tagged_ptr _TaggedPtr> [[clang::always_inline]] constexpr auto tagged_pointer_cast(typename _TaggedPtr::dirty_pointer _ptr) noexcept -> _TaggedPtr {
  return tagged_pointer_cast<typename _TaggedPtr::schema>(_ptr);
}

// conversion
template <typename _NewScheme, typename _Scheme> constexpr auto schema_pointer_cast(tagged_ptr<_Scheme> in) noexcept {
  return tagged_ptr<_NewScheme>(in.pointer(), in.tag());
}






template <typename _T, typename _Scheme> constexpr auto const_pointer_cast(tagged_ptr<_Scheme> in) noexcept {
  using new_schema = typename _Scheme::template rebind<_T>;
  using new_pointer = typename new_schema::clean_pointer;
  return tagged_ptr<new_schema>(const_cast<new_pointer>(in.pointer()), in.tag());
}

template <typename _T, typename _Scheme> constexpr auto const_static_cast(tagged_ptr<_Scheme> in) noexcept {
  using new_schema = typename _Scheme::template rebind<_T>;
  using new_pointer = typename new_schema::clean_pointer;
  return tagged_ptr<new_schema>(static_cast<new_pointer>(in.pointer()), in.tag());
}

template <typename _T, typename _Scheme> constexpr auto const_dynamic_cast(tagged_ptr<_Scheme> in) noexcept {
  using new_schema = typename _Scheme::template rebind<_T>;
  using new_pointer = typename new_schema::clean_pointer;
  return tagged_ptr<new_schema>(dynamic_cast<new_pointer>(in.pointer()), in.tag());
}

template <typename _T, typename _Scheme> auto const_reinterpret_cast(tagged_ptr<_Scheme> in) noexcept {
  using new_schema = typename _Scheme::template rebind<_T>;
  using new_pointer = typename new_schema::clean_pointer;
  return tagged_ptr<new_schema>(reinterpret_cast<new_pointer>(in.pointer()), in.tag());
}







// wrapper class containing the pointer value and provides access
template <pointer_tagging_schema _Scheme> class tagged_ptr {
public:
  using schema = _Scheme; 
  using dirty_pointer = typename schema::dirty_pointer;
  using clean_pointer = typename schema::clean_pointer;
  using tag_type = typename schema::tag_type;
  
  using value_type = std::remove_cvref_t<decltype(*std::declval<clean_pointer>())>;
  using difference_type = typename std::pointer_traits<clean_pointer>::difference_type;
  
  
  template <typename _T> using rebind = tagged_ptr<typename _Scheme::template rebind<_T>>;
  
private:
  
  dirty_pointer _pointer{nullptr};
  
  // special hidden constructor to allow constructing unsafely
  [[clang::always_inline]] constexpr tagged_ptr(schema, dirty_pointer _ptr) noexcept: _pointer{_ptr} { }
  
  friend constexpr tagged_ptr tagged_pointer_cast<schema>(dirty_pointer) noexcept;
  
  template <typename _T, typename _Scheme2> constexpr auto const_pointer_cast(tagged_ptr<_Scheme2> in) noexcept -> tagged_ptr<typename _Scheme2::template rebind<_T>>;
  
public:
  tagged_ptr() = default;
  consteval tagged_ptr(nullptr_t) noexcept: _pointer{nullptr} { }
  tagged_ptr(const tagged_ptr &) = default;
  tagged_ptr(tagged_ptr &&) = default;
  ~tagged_ptr() = default;
  tagged_ptr & operator=(const tagged_ptr &) = default;
  tagged_ptr & operator=(tagged_ptr &&) = default;
  
  [[clang::always_inline]] explicit constexpr tagged_ptr(clean_pointer _ptr, tag_type _tag = {}) noexcept: _pointer{schema::tag_pointer(_ptr, _tag)} {
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(pointer() == _ptr, "pointer must be recoverable after untagging");
    _LIBCPP_ASSERT_SEMANTIC_REQUIREMENT(tag() == _tag, "stored tag must be recoverable and within schema provided bit capacity");
  } 

  // accessors
  [[clang::always_inline]] constexpr decltype(auto) operator*() const noexcept {
    return *pointer();
  }
  
  [[clang::always_inline]] constexpr clean_pointer operator->() const noexcept {
    return pointer();
  }
   
  template <typename...Ts> [[clang::always_inline]] [[clang::always_inline]] constexpr decltype(auto) operator[](Ts... args) const noexcept requires std::is_array_v<value_type> && (sizeof...(Ts) == std::rank_v<value_type>) {
    return (*pointer())[args...];
  }
  
  [[clang::always_inline]] constexpr decltype(auto) operator[](difference_type diff) const noexcept requires (!std::is_array_v<value_type>) {
    return *(pointer() + diff);
  }
  
  // swap
  [[clang::always_inline]] friend constexpr void swap(tagged_ptr & lhs, tagged_ptr & rhs) noexcept {
    std::swap(lhs._pointer, rhs._pointer);
  }
  
  // modifiers for tag
  [[clang::always_inline]] constexpr auto & set(tag_type new_tag) noexcept {
    // this is here so I can avoid checks
    // TODO we should be able to check what bits available
    _pointer = schema::tag_pointer(pointer(), new_tag);
    return *this;
  }
  
  [[clang::always_inline]] constexpr auto & set_union(tag_type addition) noexcept {
    return set(tag() | addition);
  }
  
  [[clang::always_inline]] constexpr auto & set_difference(tag_type mask) noexcept {
    return set(tag() & (~static_cast<uintptr_t>(mask)));
  }
  
  [[clang::always_inline]] constexpr auto & set_intersection(tag_type mask) noexcept {
    return set(tag() & mask);
  }
  
  [[clang::always_inline]] constexpr auto & set_all() noexcept {
    return set(static_cast<tag_type>(0xFFFFFFFF'FFFFFFFFull));
  }

  // modifiers for pointer
  [[clang::always_inline]] constexpr auto & operator++() noexcept {
    _pointer = tagged_ptr{pointer()+1u, tag()}._pointer;
    return *this;
  }
  
  [[clang::always_inline]] constexpr auto operator++(int) noexcept {
    auto copy = auto(*this);
    this->operator++();
    return copy;
  }
  
  [[clang::always_inline]] constexpr auto & operator+=(difference_type diff) noexcept {
    _pointer = tagged_ptr{pointer()+diff, tag()}._pointer;
    return *this;
  }
  
  [[clang::always_inline]] friend constexpr auto operator+(tagged_ptr lhs, difference_type diff) noexcept {
    lhs += diff;
    return lhs;
  }
  
  [[clang::always_inline]] friend constexpr auto operator+(difference_type diff, tagged_ptr rhs) noexcept {
    rhs += diff;
    return rhs;
  }
  
  [[clang::always_inline]] friend constexpr auto operator-(tagged_ptr lhs, difference_type diff) noexcept {
    lhs -= diff;
    return lhs;
  }
  
  [[clang::always_inline]] friend constexpr auto operator-(difference_type diff, tagged_ptr rhs) noexcept {
    rhs -= diff;
    return rhs;
  }
  
  [[clang::always_inline]] constexpr auto & operator-=(difference_type diff) noexcept {
    _pointer = tagged_ptr{pointer()-diff, tag()}._pointer;
    return *this;
  }
  
  [[clang::always_inline]] constexpr auto & operator--() noexcept {
    _pointer = tagged_ptr{pointer()-1u, tag()}._pointer;
    return *this;
  }
  
  [[clang::always_inline]] constexpr auto operator--(int) noexcept {
    auto copy = auto(*this);
    this->operator--();
    return copy;
  }
  
  // observers
  dirty_pointer unsafe_dirty_pointer() const noexcept {
    // this function is not intentionally constexpr, as it is needed only to interact with
    // existing runtime code
    return _pointer;
  } 
  
  static constexpr bool support_aliasing_masking = pointer_tagging_schema_with_aliasing<schema>;
  
  [[clang::always_inline]] constexpr clean_pointer aliasing_pointer() const noexcept {
    if constexpr (support_aliasing_masking) {
      if !consteval {
        return schema::aliasing_pointer_mask(_pointer);
      }
    }
    
    return schema::pointer_mask(_pointer);
  }
  
  [[clang::always_inline]] constexpr clean_pointer pointer() const noexcept {
    return schema::pointer_mask(_pointer);
  }
  
  [[clang::always_inline]] constexpr tag_type tag() const noexcept {
    return schema::value_mask(_pointer);
  }
  
  template <std::size_t I> [[nodiscard, clang::always_inline]] friend constexpr decltype(auto) get(tagged_ptr _pair) noexcept {
    static_assert(I < 3);
    if constexpr (I == 0) {
      return _pair.pointer();
    } else {
      return _pair.tag();
    }
  }
  
  [[clang::always_inline]] constexpr explicit operator bool() const noexcept {
    return pointer() != nullptr;
  }
  
  [[clang::always_inline]] friend constexpr ptrdiff_t operator-(tagged_ptr lhs, tagged_ptr rhs) noexcept {
    return lhs.pointer() - rhs.pointer();
  }
  
  // comparison operators
  [[clang::always_inline]] friend bool operator==(tagged_ptr, tagged_ptr) = default;
  
  struct _compare_object {
    clean_pointer pointer;
    tag_type tag;
    
    friend auto operator<=>(_compare_object, _compare_object) = default;
  };
  
  [[clang::always_inline]] friend constexpr auto operator<=>(tagged_ptr lhs, tagged_ptr rhs) noexcept {
    return _compare_object{lhs.pointer(), lhs.tag()} <=> _compare_object{rhs.pointer(), rhs.tag()};
  }
  [[clang::always_inline]] friend constexpr bool operator==(tagged_ptr lhs, clean_pointer rhs) noexcept {
    return lhs.pointer() == rhs;
  }
  [[clang::always_inline]] friend constexpr auto operator<=>(tagged_ptr lhs, clean_pointer rhs) noexcept {
    return lhs.pointer() <=> rhs;
  }
  [[clang::always_inline]] friend constexpr bool operator==(tagged_ptr lhs, nullptr_t) noexcept {
    return lhs.pointer() == nullptr;
  }
};

// to_address specialization
template <typename _Scheme> static constexpr auto to_address(tagged_ptr<_Scheme> p) noexcept -> tagged_ptr<_Scheme>::element_type * {
  return p.pointer();
}

// iterator traits
template <typename _Scheme>
struct _LIBCPP_TEMPLATE_VIS iterator_traits<tagged_ptr<_Scheme>> {
  using _tagged_ptr = tagged_ptr<_Scheme>;
  
  using iterator_category = std::random_access_iterator_tag;
  using iterator_concept = std::contiguous_iterator_tag;
  
  using value_type = _tagged_ptr::value_type;
  using reference = value_type &;
  using pointer = _tagged_ptr::clean_pointer;
  using difference_type = tagged_ptr<_Scheme>::difference_type;
};

// pointer traits
template <typename _Scheme>
struct _LIBCPP_TEMPLATE_VIS pointer_traits<tagged_ptr<_Scheme>> {
  using _tagged_ptr = tagged_ptr<_Scheme>;
  using pointer = _tagged_ptr::clean_pointer;
  using element_type = _tagged_ptr::value_type;
  using difference_type = _tagged_ptr::difference_type;
  
  // what to do with this?
  template <typename _Up> using rebind = typename _tagged_ptr::template rebind<_Up>;

public:
  _LIBCPP_HIDE_FROM_ABI constexpr static pointer pointer_to(pointer ptr) _NOEXCEPT {
    return tagged_ptr<_Scheme>{ptr};
  }
};

// we are defaulting always to low_bits schema
template <typename _T> tagged_ptr(_T *) -> tagged_ptr<alignment_low_bits_tag<_T*>>;
template <typename _T> tagged_ptr(_T *, uintptr_t) -> tagged_ptr<alignment_low_bits_tag<_T*>>;

// support for tuple protocol so we can split tagged pointer to structured bindings:
// auto [ptr, tag] = tagged_ptr
template <typename _Scheme>
struct tuple_size<tagged_ptr<_Scheme>>: std::integral_constant<std::size_t, 2> {};

template <std::size_t I, typename _Scheme>
struct tuple_element<I, tagged_ptr<_Scheme>> {
  using _pair_type = tagged_ptr<_Scheme>;
  using type = std::conditional_t<I == 0, typename _pair_type::clean_pointer, typename _pair_type::tag_type>;
};

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP_STD_VER >= 26

#endif // _LIBCPP___TAGGED_POINTER_H
