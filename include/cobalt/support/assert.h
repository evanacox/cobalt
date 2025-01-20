//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2025 Evan Cox <evanacox00@gmail.com>. All rights reserved.      //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//

#pragma once

#include "cobalt/support/clang.h"
#include <source_location>
#include <string_view>

namespace cobalt::internal {

struct AssertResult {
  std::string_view condition;
  std::string_view explanation;
};

[[noreturn]] void assert_fail(AssertResult result,
    std::source_location loc = std::source_location::current()) noexcept;

[[noreturn]] void hit_unreachable(std::string_view explanation,
    std::source_location loc = std::source_location::current()) noexcept;

} // namespace cobalt::internal

#ifdef COBALT_DEBUG
#define COBALT_ASSERT(condition, reason)                                                           \
  do {                                                                                             \
    if (!(condition)) [[unlikely]] {                                                               \
      ::cobalt::internal::assert_fail(COBALT_STRINGIFY(condition), reason);                        \
    }                                                                                              \
  } while (false)
#define COBALT_UNREACHABLE(reason) ::cobalt::internal::hit_unreachable(reason)
#else
#define COBALT_ASSERT(condition, reason) ::cobalt::assume((condition))
#define COBALT_UNREACHABLE(reason) ::cobalt::unreachable()
#endif