//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2025 Evan Cox <evanacox00@gmail.com>. All rights reserved.      //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//

#include "cobalt/utility/arena.h"
#include "cobalt/support/assert.h"

namespace cobalt {

namespace internal {

void AllocatorStatsEnabled::record_alloc(Arena* arena, usize size, AllocKind kind) noexcept {
  //
}

} // namespace internal

byte* Arena::alloc(usize size, AllocKind kind) noexcept {
  COBALT_UNREACHABLE("todo");
}

} // namespace cobalt