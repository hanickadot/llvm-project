//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___EXCEPTION_EXCEPTION_PTR_H
#define _LIBCPP___EXCEPTION_EXCEPTION_PTR_H

#include <__config>
#include <__exception/operations.h>
#include <__memory/addressof.h>
#include <__memory/construct_at.h>
#include <__type_traits/decay.h>
#include <cstddef>
#include <cstdlib>
#include <new>
#include <typeinfo>
#include <cxxabi.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#ifndef _LIBCPP_ABI_MICROSOFT

#  if _LIBCPP_AVAILABILITY_HAS_INIT_PRIMARY_EXCEPTION

namespace __cxxabiv1 {

extern "C" {
_LIBCPP_OVERRIDABLE_FUNC_VIS void* __cxxabiv1::__cxa_allocate_exception(size_t) throw();
_LIBCPP_OVERRIDABLE_FUNC_VIS void __cxxabiv1::__cxa_free_exception(void*) throw();

struct __cxa_exception;
_LIBCPP_OVERRIDABLE_FUNC_VIS __cxxabiv1::__cxa_exception* __cxxabiv1::__cxa_init_primary_exception(
    void*,
    std::type_info*,
#    if defined(_WIN32)
    void(__thiscall*)(void*)) throw();
#    elif defined(__wasm__)
    // In Wasm, a destructor returns its argument
    void* (*)(void*)) throw();
#    else
    void (*)(void*)) throw();
#    endif
}

} // namespace __cxxabiv1

#  endif

#endif

namespace std { // purposefully not using versioning namespace

#ifndef _LIBCPP_ABI_MICROSOFT

class _LIBCPP_EXPORTED_FROM_ABI exception_ptr {
  void* __ptr_;

  static constexpr exception_ptr __from_native_exception_pointer(void*) _NOEXCEPT;

  template <class _Ep>
  constexpr friend _LIBCPP_HIDE_FROM_ABI exception_ptr make_exception_ptr(_Ep) _NOEXCEPT;

public:
  constexpr _LIBCPP_HIDE_FROM_ABI exception_ptr() _NOEXCEPT : __ptr_() {}
  constexpr _LIBCPP_HIDE_FROM_ABI exception_ptr(nullptr_t) _NOEXCEPT : __ptr_() {}

  constexpr exception_ptr(const exception_ptr&) _NOEXCEPT;
  constexpr exception_ptr& operator=(const exception_ptr&) _NOEXCEPT;
  constexpr ~exception_ptr() _NOEXCEPT;

  _LIBCPP_HIDE_FROM_ABI explicit operator bool() const _NOEXCEPT { return __ptr_ != nullptr; }

  constexpr friend _LIBCPP_HIDE_FROM_ABI bool operator==(const exception_ptr& __x, const exception_ptr& __y) _NOEXCEPT {
    return __x.__ptr_ == __y.__ptr_;
  }

  constexpr friend _LIBCPP_HIDE_FROM_ABI bool operator!=(const exception_ptr& __x, const exception_ptr& __y) _NOEXCEPT {
    return !(__x == __y);
  }

  constexpr friend _LIBCPP_EXPORTED_FROM_ABI exception_ptr current_exception() _NOEXCEPT;
  constexpr friend _LIBCPP_EXPORTED_FROM_ABI void rethrow_exception(exception_ptr);
};

template <class _Ep>
constexpr _LIBCPP_HIDE_FROM_ABI exception_ptr make_exception_ptr(_Ep __e) _NOEXCEPT {
  if consteval {
    try {
      throw __e;
    } catch (...) {
      return current_exception();
    }
  } else {
#  ifndef _LIBCPP_HAS_NO_EXCEPTIONS
#    if _LIBCPP_AVAILABILITY_HAS_INIT_PRIMARY_EXCEPTION && __cplusplus >= 201103L
  using _Ep2 = __decay_t<_Ep>;

  void* __ex = __cxxabiv1::__cxa_allocate_exception(sizeof(_Ep));
#      ifdef __wasm__
  // In Wasm, a destructor returns its argument
  (void)__cxxabiv1::__cxa_init_primary_exception(__ex, const_cast<std::type_info*>(&typeid(_Ep)), [](void* __p) -> void* {
#      else
  (void)__cxxabiv1::__cxa_init_primary_exception(__ex, const_cast<std::type_info*>(&typeid(_Ep)), [](void* __p) {
#      endif
    std::__destroy_at(static_cast<_Ep2*>(__p));
#      ifdef __wasm__
    return __p;
#      endif
  });

  try {
    ::new (__ex) _Ep2(__e);
    return exception_ptr::__from_native_exception_pointer(__ex);
  } catch (...) {
    __cxxabiv1::__cxa_free_exception(__ex);
    return current_exception();
  }
#    else
  try {
    throw __e;
  } catch (...) {
    return current_exception();
  }
#    endif
#  else
  ((void)__e);
  std::abort();
#  endif
  }
}

constexpr exception_ptr::~exception_ptr() noexcept { 
  if consteval {
    __builtin_constexpr_exception_refcount_dec(__ptr_);
  } else {
    __cxxabiv1::__cxa_decrement_exception_refcount(__ptr_);
  }
}

constexpr exception_ptr::exception_ptr(const exception_ptr& other) noexcept : __ptr_(other.__ptr_) {
  if consteval {
     __builtin_constexpr_exception_refcount_inc(__ptr_);
  } else {
    __cxxabiv1::__cxa_increment_exception_refcount(__ptr_);
  }
}

constexpr exception_ptr& exception_ptr::operator=(const exception_ptr& other) noexcept {
  if (__ptr_ != other.__ptr_) {
    if consteval {
      __builtin_constexpr_exception_refcount_inc(other.__ptr_);
      __builtin_constexpr_exception_refcount_dec(__ptr_);
    } else {
      __cxxabiv1::__cxa_increment_exception_refcount(other.__ptr_);
      __cxxabiv1::__cxa_decrement_exception_refcount(__ptr_);
      __ptr_ = other.__ptr_;
    }
  }
  return *this;
}

constexpr exception_ptr exception_ptr::__from_native_exception_pointer(void* __e) noexcept {
  exception_ptr ptr;
  ptr.__ptr_ = __e;
  if consteval {
  __builtin_constexpr_exception_refcount_inc(ptr.__ptr_);
  } else {
    __cxxabiv1::__cxa_increment_exception_refcount(ptr.__ptr_);
  }

  return ptr;
}

constexpr exception_ptr current_exception() noexcept {
  // be nicer if there was a constructor that took a ptr, then
  // this whole function would be just:
  //    return exception_ptr(__cxxabiv1::__cxa_current_primary_exception());
  exception_ptr ptr;
  if consteval {
    ptr.__ptr_ = __builtin_constexpr_current_exception();
    __builtin_constexpr_exception_refcount_inc(ptr.__ptr_);
  } else {
    ptr.__ptr_ = __cxxabiv1::__cxa_current_primary_exception();
  }
  return ptr;
}

_LIBCPP_NORETURN constexpr void rethrow_exception(exception_ptr p) {
  if consteval {
    __builtin_constexpr_rethrow_exception(p.__ptr_);
  } else {
    __cxxabiv1::__cxa_rethrow_primary_exception(p.__ptr_);
    // if p.__ptr_ is NULL, above returns so we terminate
  }
  __builtin_abort();
}

#else // _LIBCPP_ABI_MICROSOFT

class _LIBCPP_EXPORTED_FROM_ABI exception_ptr {
  _LIBCPP_DIAGNOSTIC_PUSH
  _LIBCPP_CLANG_DIAGNOSTIC_IGNORED("-Wunused-private-field")
  void* __ptr1_;
  void* __ptr2_;
  _LIBCPP_DIAGNOSTIC_POP

public:
  exception_ptr() _NOEXCEPT;
  exception_ptr(nullptr_t) _NOEXCEPT;
  exception_ptr(const exception_ptr& __other) _NOEXCEPT;
  exception_ptr& operator=(const exception_ptr& __other) _NOEXCEPT;
  exception_ptr& operator=(nullptr_t) _NOEXCEPT;
  ~exception_ptr() _NOEXCEPT;
  explicit operator bool() const _NOEXCEPT;
};

_LIBCPP_EXPORTED_FROM_ABI bool operator==(const exception_ptr& __x, const exception_ptr& __y) _NOEXCEPT;

inline _LIBCPP_HIDE_FROM_ABI bool operator!=(const exception_ptr& __x, const exception_ptr& __y) _NOEXCEPT {
  return !(__x == __y);
}

_LIBCPP_EXPORTED_FROM_ABI void swap(exception_ptr&, exception_ptr&) _NOEXCEPT;

_LIBCPP_EXPORTED_FROM_ABI exception_ptr __copy_exception_ptr(void* __except, const void* __ptr);
_LIBCPP_EXPORTED_FROM_ABI exception_ptr current_exception() _NOEXCEPT;
_LIBCPP_NORETURN _LIBCPP_EXPORTED_FROM_ABI void rethrow_exception(exception_ptr);

// This is a built-in template function which automagically extracts the required
// information.
template <class _E>
void* __GetExceptionInfo(_E);

template <class _Ep>
_LIBCPP_HIDE_FROM_ABI exception_ptr make_exception_ptr(_Ep __e) _NOEXCEPT {
  return __copy_exception_ptr(std::addressof(__e), __GetExceptionInfo(__e));
}

#endif // _LIBCPP_ABI_MICROSOFT
} // namespace std

#endif // _LIBCPP___EXCEPTION_EXCEPTION_PTR_H
