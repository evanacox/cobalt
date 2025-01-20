//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2025 Evan Cox <evanacox00@gmail.com>. All rights reserved.      //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//

#include "cobalt/support/assert.h"
#include "cobalt/support/clang.h"
#include <iostream>
#include <source_location>
#include <string_view>

namespace cobalt {

void internal::assert_fail(AssertResult result, std::source_location loc) noexcept {
  std::cerr << "[cobalt] assertion failure! (in '" << loc.function_name() << "' at '"
            << loc.file_name() << ":" << loc.line() << "')\n"
            << "    condition: '" << result.condition << "'\n"
            << "    explanation: '" << result.explanation << "'\n";

  cobalt::trap();
}

void internal::hit_unreachable(std::string_view explanation, std::source_location loc) noexcept {
  std::cerr << "[cobalt] hit unreachable code! (in '" << loc.function_name() << "' at '"
            << loc.file_name() << ":" << loc.line() << "')\n"
            << "    explanation: '" << explanation << "'\n";

  cobalt::trap();
}

} // namespace cobalt