//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <__config>
#ifdef _LIBCPP_DEPRECATED_ABI_LEGACY_LIBRARY_DEFINITIONS_FOR_INLINE_FUNCTIONS
#  define _LIBCPP_SHARED_PTR_DEFINE_LEGACY_INLINE_FUNCTIONS
#endif

#include <memory>

#ifndef _LIBCPP_HAS_NO_THREADS
#  include <mutex>
#  include <thread>
#  if defined(__ELF__) && defined(_LIBCPP_LINK_PTHREAD_LIB)
#    pragma comment(lib, "pthread")
#  endif
#endif

#include "include/atomic_support.h"

_LIBCPP_BEGIN_NAMESPACE_STD
  
void __shared_weak_count::__release_weak() _NOEXCEPT {
  __release_weak_internal();
}

#if !defined(_LIBCPP_HAS_NO_THREADS)

static constexpr std::size_t __sp_mut_count                = 32;
static constinit __libcpp_mutex_t mut_back[__sp_mut_count] = {
    _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER,
    _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER,
    _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER,
    _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER,
    _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER,
    _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER,
    _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER,
    _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER, _LIBCPP_MUTEX_INITIALIZER};

constexpr __sp_mut::__sp_mut(void* p) noexcept : __lx_(p) {}

void __sp_mut::lock() noexcept {
  auto m = static_cast<__libcpp_mutex_t*>(__lx_);
  __libcpp_mutex_lock(m);
}

void __sp_mut::unlock() noexcept { __libcpp_mutex_unlock(static_cast<__libcpp_mutex_t*>(__lx_)); }

__sp_mut& __get_sp_mut(const void* p) {
  static constinit __sp_mut muts[__sp_mut_count] = {
      &mut_back[0],  &mut_back[1],  &mut_back[2],  &mut_back[3],  &mut_back[4],  &mut_back[5],  &mut_back[6],
      &mut_back[7],  &mut_back[8],  &mut_back[9],  &mut_back[10], &mut_back[11], &mut_back[12], &mut_back[13],
      &mut_back[14], &mut_back[15], &mut_back[16], &mut_back[17], &mut_back[18], &mut_back[19], &mut_back[20],
      &mut_back[21], &mut_back[22], &mut_back[23], &mut_back[24], &mut_back[25], &mut_back[26], &mut_back[27],
      &mut_back[28], &mut_back[29], &mut_back[30], &mut_back[31]};
  return muts[hash<const void*>()(p) & (__sp_mut_count - 1)];
}

#endif // !defined(_LIBCPP_HAS_NO_THREADS)

void* align(size_t alignment, size_t size, void*& ptr, size_t& space) {
  void* r = nullptr;
  if (size <= space) {
    char* p1 = static_cast<char*>(ptr);
    char* p2 = reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(p1 + (alignment - 1)) & -alignment);
    size_t d = static_cast<size_t>(p2 - p1);
    if (d <= space - size) {
      r   = p2;
      ptr = r;
      space -= d;
    }
  }
  return r;
}

_LIBCPP_END_NAMESPACE_STD
