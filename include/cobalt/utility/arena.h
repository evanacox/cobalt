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
#include "cobalt/support/macros.h"
#include "cobalt/support/types.h"
#include "cobalt/utility/concepts.h"
#include <concepts>
#include <utility>

namespace cobalt {

enum class AllocKind {

};

class LinearArena;

namespace internal {

class AllocatorStatsEnabled {
public:
  void record_alloc(LinearArena* arena, usize size, AllocKind kind) noexcept;
};

class AllocatorStatsDisabled {
public:
  COBALT_ALWAYS_INLINE void record_alloc(LinearArena* /*unused*/,
      usize /*unused*/,
      AllocKind /*unused*/) noexcept {}
};

#ifdef COBALT_DEBUG
using AllocatorStats = AllocatorStatsEnabled;
#else
using AllocatorStats = AllocatorStatsDisabled;
#endif

} // namespace internal

/// An RAII type that encodes a "frame" of the Arena.
///
/// Everything allocated after the frame is created will be marked as unused
/// whenever this object is destructed.
class ArenaFrame {
public:
  ArenaFrame(ArenaFrame&& other) noexcept
      : arena_{std::exchange(other.arena_, nullptr)},
        old_current_{std::exchange(other.old_current_, nullptr)} {}

  ArenaFrame& operator=(ArenaFrame&& other) noexcept {
    arena_ = std::exchange(other.arena_, nullptr);
    old_current_ = std::exchange(other.old_current_, nullptr);

    return *this;
  }

  inline ~ArenaFrame() noexcept;

protected:
  COBALT_DISALLOW_COPY_AND_ASSIGN(ArenaFrame);

private:
  friend class LinearArena;

  explicit ArenaFrame(LinearArena* arena, byte* old_current) noexcept
      : arena_{arena},
        old_current_{old_current} {}

  LinearArena* arena_;
  byte* old_current_;
};

/// An arena that uses linear allocation for extremely fast
/// bulk allocation/deallocation.
class LinearArena : public internal::AllocatorStats {
public:
  ///
  ///
  /// \param size
  /// \param kind
  /// \return
  [[nodiscard]] byte* alloc_size(usize size, AllocKind kind) noexcept {
    //
  }

  [[nodiscard]] ArenaFrame enter_frame() noexcept {
    return ArenaFrame{this, current_};
  }

private:
  friend class ArenaFrame;

  void leave_frame(byte* previous_current) noexcept {
    current_ = previous_current;
  }

  byte* base_;
  byte* current_;
  usize used_;
  usize total_size_;
};

inline ArenaFrame::~ArenaFrame() noexcept {
  arena_->leave_frame(old_current_);
}

} // namespace cobalt