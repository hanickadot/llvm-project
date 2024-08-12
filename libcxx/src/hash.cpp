//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <__hash_table>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

_LIBCPP_CLANG_DIAGNOSTIC_IGNORED("-Wtautological-constant-out-of-range-compare")

_LIBCPP_BEGIN_NAMESPACE_STD
  
_LIBCPP_EXPORTED_FROM_ABI size_t __next_prime(size_t n) {
  return std::hidden::__next_prime(n);
}

_LIBCPP_END_NAMESPACE_STD
