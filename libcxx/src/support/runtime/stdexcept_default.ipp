//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

/* For _LIBCPPABI_VERSION */
#if !defined(_LIBCPP_BUILDING_HAS_NO_ABI_LIBRARY) && (defined(LIBCXX_BUILDING_LIBCXXABI) || defined(LIBCXXRT))
#  include <cxxabi.h>
#endif


