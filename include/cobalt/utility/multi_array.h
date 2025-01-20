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
// This file defines several utility wrappers for `absl::InlinedVector` that //
// are used within the project. These model common patterns within the       //
// project, and make them easy to reuse rather than re-implementing the same //
// solution many times.                                                      //
//                                                                           //
//======---------------------------------------------------------------======//

#pragma once

#include "absl/container/inlined_vector.h"
#include "cobalt/support/types.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>

namespace cobalt {

/// A container that compresses multiple vectors into one contiguous array plus some indices.
///
/// This has exactly one major use case: dealing with types that contain multiple `vec(T)` objects
/// within the WASM bytecode. The rest of this should be read with that context.
///
/// To understand this, consider the following example arrays:
///
///           vec1 = [1, 2, 3]
///           vec2 = [4, 5]
///           vec3 = [6, 7, 8, 9, 10]
///
/// This utility type represents this as the following single array:
///
///           [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
///            ^        ^     ^
///            vec1     vec2  vec3
///
/// This helps the cache (especially for very small types) due to the increased data density,
/// and it also reduces overhead because the ~24 bytes of overhead (ignoring small-size
/// optimization) per vector is reduced to just a single index per vector plus the
/// overhead for one.
///
/// Obviously, this requires giving up easy mutation on any individual sub-vector, but
/// in a (practically) read-only context like this, that's perfectly fine. This API provides
/// an easy way to build the arrays the first time, and once that's done, mutation (except
/// per-element) is done too. This is all we need for parsing the bytecode: we read each one
/// in order, and we build the underlying array as we read each individual `vec(T)` from
/// the WASM module.
///
/// \tparam T The type being held in each vector
/// \tparam N The number of sub-arrays
/// \tparam SmallSize A suggested small size for SSO. This should be the size of **all** sub-vectors
template <typename T, usize N, usize SmallSize> class SmallMultiArray {
public:
  SmallMultiArray() = delete;

  /// Gets the `i`th sub-array from the container.
  ///
  /// \param i The sub-array to get
  /// \return A span pointing to the sub-array
  [[nodiscard]] std::span<const T> sub_vec(usize i) const noexcept {
    assert(i < N && "attempted to get non-existent sub-vec");

    auto begin = (i == 0) ? 0 : indices_[i - 1];
    auto length = indices_[i] - begin;

    return std::span{array_.data() + begin, length};
  }

  /// Gets the `i`th sub-array from the container.
  ///
  /// \param i The sub-array to get
  /// \return A span pointing to the sub-array
  [[nodiscard]] std::span<T> sub_vec(usize i) noexcept {
    assert(i < N && "attempted to get non-existent sub-vec");

    auto begin = (i == 0) ? 0 : indices_[i - 1];
    auto length = indices_[i] - begin;

    return std::span{array_.data() + begin, length};
  }

protected:
  template <typename U, usize M, usize S> friend class MultiArrayBuilder;

  SmallMultiArray(absl::InlinedVector<T, N> data, std::array<u32, N> indices)
      : array_{std::move(data)},
        indices_{indices} {}

private:
  // indices_, for each sub-array `i`, contains the end index at indices_[i].
  // this means that indices_[N - 1] == array_.size()
  absl::InlinedVector<T, N> array_;
  std::array<u32, N> indices_;
};

/// Helper to correctly build multi-arrays. Use the methods, then call `.build()`.
///
/// \tparam T See MultiArray's T
/// \tparam N See MultiArray's N
/// \tparam SmallSize See MultiArray's SmallSize
template <typename T, usize N, usize SmallSize> class MultiArrayBuilder {
public:
  void push(T value) noexcept {
    underlying_.push_back(std::move(value));
  }

  void end_sub_vec() noexcept {
    assert(current_index_ < N && "out of bounds on MultiArrayBuilder");

    indices_[current_index_++] = underlying_.size();
  }

  [[nodiscard]] SmallMultiArray<T, N, SmallSize> build() noexcept {
    return MultiArray(std::move(underlying_), indices_);
  }

private:
  absl::InlinedVector<T, N> underlying_;
  std::array<u32, N> indices_ = {0};
  usize current_index_ = 0;
};

} // namespace cobalt