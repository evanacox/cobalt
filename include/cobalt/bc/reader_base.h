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
// This file contains the WasmBytecodeReaderBase class, a base type for any  //
// code that needs to parse WASM bytecode.                                   //
//                                                                           //
// It is designed like this (as a CRTP base class) for a specific reason:    //
// inlining and specialization. A user could derive this and override the    //
// specialization points to turn it into a single-pass WASM compiler, and    //
// in an optimizing build there would be proper inlining and whatnot without //
// any extra effort required.                                                //
//                                                                           //
// Everything is already templated and available in the header, so any user  //
// can get as little overhead as possible in their special cases.            //
//                                                                           //
//======---------------------------------------------------------------======//

#ifndef COBALT_READER_READER_BASE_H
#define COBALT_READER_READER_BASE_H

#include "absl/container/inlined_vector.h"
#include "absl/strings/str_cat.h"
#include "cobalt/ast/types.h"
#include "cobalt/support/clang.h"
#include "cobalt/support/exceptions.h"
#include "cobalt/support/types.h"
#include "gtest/gtest_prod.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace cobalt {

/// An exception while reading WASM bytecode. This means the input
/// was malformed in some way, typically.
class WasmBytecodeReadException : public ValidationFailure {
public:
  explicit WasmBytecodeReadException(const char* message)
      : ValidationFailure{std::string_view{message}} {}

  explicit WasmBytecodeReadException(std::string message) : ValidationFailure{std::move(message)} {}
};

namespace internal {

struct LEB128ReaderResult {
  u64 result;
  u64 shift;
  u64 last_byte;
};

} // namespace internal

/// A base class for any type that wants to read WASM binaries. It is designed
/// to allow template polymorphism for derived instances.
///
/// It's a standard CRTP base class, allowing derived instances to override exactly
/// what behavior the parser has, enabling true single-pass compilation for example.
template <typename Derived> class WasmBytecodeReaderBase {
public:
  explicit WasmBytecodeReaderBase(std::span<u8> bytecode) : remaining_{bytecode} {}

  void read() {
    //
  }

  [[nodiscard]] u8 consume() {
    if (remaining_.empty()) [[unlikely]] {
      throw WasmBytecodeReadException{"unexpected end of module"};
    }

    auto curr = remaining_[0];

    remaining_ = remaining_.subspan(1);

    return curr;
  }

  void expect(u8 value) {
    auto got = consume();

    if (got != value) [[unlikely]] {
      throw WasmBytecodeReadException{absl::StrCat("got unexpected byte, expected '",
          absl::Hex(value),
          "' but got '",
          absl::Hex(got),
          "'")};
    }
  }

  template <int N> [[nodiscard]] std::array<u8, N> consume_n() {
    auto buf = std::array<u8, N>{};

    for (auto i = 0; i < N; ++i) {
      buf[i] = consume();
    }

    return buf;
  }

  // defined in internal/types.h

  [[nodiscard]] ValueType read_val_ty();

  [[nodiscard]] absl::InlinedVector<ValueType, 4> read_result_ty();

  [[nodiscard]] FunctionType read_function_ty();

  [[nodiscard]] Limit read_limit();

  [[nodiscard]] MemoryType read_memory_ty();

  [[nodiscard]] TableType read_table_ty();

  [[nodiscard]] GlobalType read_global_ty();

  // defined in internal/values.h

  [[nodiscard]] float read_f32();

  [[nodiscard]] double read_f64();

  [[nodiscard]] u8 read_byte();

  [[nodiscard]] u8 read_u8();

  [[nodiscard]] u16 read_u16();

  [[nodiscard]] u32 read_u32();

  [[nodiscard]] u64 read_u64();

  [[nodiscard]] i8 read_i8();

  [[nodiscard]] i16 read_i16();

  [[nodiscard]] i32 read_i32();

  [[nodiscard]] i64 read_i64();

  template <typename T> using ReadMethod = T (WasmBytecodeReaderBase<Derived>::*)();

  template <typename T> [[nodiscard]] absl::InlinedVector<T, 4> read_vec(ReadMethod<T> reader);

  [[nodiscard]] std::string read_name();

  template <int N> [[nodiscard]] COBALT_ALWAYS_INLINE u64 read_leb128_unsigned();

  template <int N> [[nodiscard]] COBALT_ALWAYS_INLINE i64 read_leb128_signed();

  template <int N>
  [[nodiscard]] COBALT_ALWAYS_INLINE internal::LEB128ReaderResult read_leb128_internal();

private:
  std::span<u8> remaining_;
};

} // namespace cobalt

// out-of-line template definitions for many of these, they need
// to be excluded from the endif to avoid recursive includes
#include "cobalt/bc/internal/types.h"
#include "cobalt/bc/internal/values.h"

#endif
