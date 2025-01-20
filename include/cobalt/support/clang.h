//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2025 Evan Cox <evanacox00@gmail.com>. All rights reserved.      //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//
//                                                                           //
// This file contains definitions for Clang-specific stuff used within the   //
// project. These are strongly typed and have proper IDE support because of  //
// that, it also provides one single place to check if a builtin is          //
// properly supported by the compiler.                                       //
//                                                                           //
//======---------------------------------------------------------------======//

#pragma once

#include "cobalt/support/types.h"
#include <array>
#include <cstddef>

#define COBALT_NEVER_INLINE __attribute__((noinline))
#define COBALT_PURE __attribute__((const))
#define COBALT_COLD __attribute__((cold))
#define COBALT_UNLIKELY(expr) __builtin_expect((expr), false)
#define COBALT_LIKELY(expr) __builtin_expect((expr), true)

#ifdef COBALT_DEBUG
#define COBALT_ALWAYS_INLINE inline
#else
#define COBALT_ALWAYS_INLINE __attribute__((always_inline)) inline
#endif

#define COBALT_STRINGIFY1(s) #s
#define COBALT_STRINGIFY(s) COBALT_STRINGIFY1(s)

namespace cobalt {

///
using uint128 = unsigned __int128;

///
using int128 = signed __int128;

/// A `memcpy` operation that is guaranteed to be completed without any
/// external function calls. This is mostly just for `memcpy` with known
/// bounds.
///
/// \tparam N The number of bytes being copied
/// \param dest The destination array to copy into
/// \param src The source array being copied from
template <std::size_t N>
COBALT_ALWAYS_INLINE void memcpy_inline(std::array<byte, N>* dest, const byte* src) noexcept {
  __builtin_memcpy_inline(dest, src, N);
}

/// Immediately traps, and exits the program abnormally.
///
/// This does not call destructors and simply exits immediately, usually
/// triggering a fault.
[[noreturn]] COBALT_ALWAYS_INLINE void trap() noexcept {
  __builtin_trap();
}

/// Assumes that a condition is true. If it isn't at runtime,
/// the behavior of the program is undefined.
///
/// \param condition The condition assumed to be true.
COBALT_ALWAYS_INLINE void assume(bool condition) noexcept {
  __builtin_assume(condition);
}

/// An unreachable function, informs the compiler that whatever code path
/// called this function will never be executed. If it is at runtime,
/// the behavior of the program is undefined.
[[noreturn]] COBALT_ALWAYS_INLINE void unreachable() noexcept {
  __builtin_unreachable();
}

} // namespace cobalt