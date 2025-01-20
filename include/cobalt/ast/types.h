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
#include "cobalt/utility/multi_array.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>

namespace cobalt {

/// An enum for each of the different WASM value types.
///
/// These magic numbers are all from the WASM spec, see
/// https://webassembly.github.io/spec/core/binary/types.html#number-types.
enum class ValueType : std::uint8_t {
  i32 = 0x7F,
  i64 = 0x7E,
  f32 = 0x7D,
  f64 = 0x7C,
  v128 = 0x7B,
  func_ref = 0x70,
  extern_ref = 0x6F,
};

/// A reference type, which is a subset of possible value types.
///
/// See https://webassembly.github.io/spec/core/binary/types.html#reference-types.
enum class RefType : std::uint8_t {
  func_ref = 0x70,
  extern_ref = 0x6F,
};

/// Checked conversion from a `ValueType` into a `RefType`.
///
/// \param ty The value type being converted
/// \return The equivalent reference type
[[nodiscard]] FWASM_ALWAYS_INLINE RefType into_ref_ty(ValueType ty) noexcept {
  assert(ty == ValueType::func_ref
         || ty == ValueType::extern_ref && "attempting to convert non-ref to ref");

  return static_cast<RefType>(ty);
}

/// Checked conversion from a `ValueType` into a `RefType`.
///
/// \param ty The value type being converted
/// \return The equivalent reference type
[[nodiscard]] FWASM_ALWAYS_INLINE std::optional<RefType> try_into_ref_ty(ValueType ty) noexcept {
  return (ty == ValueType::func_ref || ty == ValueType::extern_ref)
             ? std::make_optional(static_cast<RefType>(ty))
             : std::nullopt;
}

/// Models the type of a function.
///
/// This has a set of parameters, and returns a set of results.
class FunctionType {
public:
  /// Returns the list of parameter types for the function.
  ///
  /// \return The list of parameter types for the function
  [[nodiscard]] std::span<const ValueType> param_tys() const noexcept {
    return vec_.sub_vec(0);
  }

  /// Returns the list of result types for the function.
  ///
  /// \return The list of result types for the function
  [[nodiscard]] std::span<const ValueType> result_tys() const noexcept {
    return vec_.sub_vec(1);
  }

private:
  SmallMultiArray<ValueType, 2, 16> vec_;
};

/// Models the size range of a memory type or table.
///
/// Either bounded or unbounded, depending on whether `.max()` returns
/// a `std::nullopt` or a value.
class Limit {
public:
  [[nodiscard]] static Limit unbounded(std::uint32_t min) noexcept {
    return Limit{min, std::numeric_limits<std::uint32_t>::max()};
  }

  [[nodiscard]] static Limit bounded(std::uint32_t min, std::uint32_t max) noexcept {
    return Limit{min, max};
  }

  /// Gets the minimum value of the limit
  ///
  /// \return The minimum value
  [[nodiscard]] std::uint32_t min() const noexcept {
    return min_;
  }

  /// Gets the maximum value, if one exists. If it doesn't, returns
  /// an empty optional.
  ///
  /// \return The maximum value, if it exists.
  [[nodiscard]] std::optional<std::uint32_t> max() const noexcept {
    return (max_ == std::numeric_limits<std::uint32_t>::max()) ? std::nullopt
                                                               : std::make_optional(max_);
  }

protected:
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  Limit(std::uint32_t min, std::uint32_t max) noexcept : min_{min}, max_{max} {}

private:
  std::uint32_t min_;
  std::uint32_t max_;
};

/// Models a linear memory type.
///
/// Contains a limit on the lower and upper bounds of the region's size,
/// measured in pages.
class MemoryType {
public:
  explicit MemoryType(Limit limit) noexcept : limits_{limit} {}

  /// Gets the size limits of the memory range, measured in pages.
  ///
  /// \return The limits
  [[nodiscard]] Limit page_limits() const noexcept {
    return limits_;
  }

private:
  Limit limits_;
};

/// Models a table containing opaque references. The size is bounded by
/// a given limit.
class TableType {
public:
  explicit TableType(Limit limit, RefType ref_type) noexcept : limits_{limit}, ty_{ref_type} {}

  /// Gets the size limits of the memory range, measured in elements.
  ///
  /// \return The limits
  [[nodiscard]] Limit page_limits() const noexcept {
    return limits_;
  }

  /// Gets the reference type being held in the table.
  ///
  /// \return The reference type
  [[nodiscard]] RefType ty() const noexcept {
    return ty_;
  }

private:
  Limit limits_;
  RefType ty_;
};

/// The type of a global. It contains a value type, and a mutability flag.
class GlobalType {
public:
  explicit GlobalType(ValueType ty, bool mut) noexcept : ty_{ty}, mut_{mut} {}

  /// The type of the global
  ///
  /// \return The value type
  [[nodiscard]] ValueType ty() const noexcept {
    return ty_;
  }

  /// Whether or not the global is mutable
  ///
  /// \return The globals' `mut` flag
  [[nodiscard]] bool mut() const noexcept {
    return mut_;
  }

private:
  ValueType ty_;
  bool mut_;
};

} // namespace cobalt
