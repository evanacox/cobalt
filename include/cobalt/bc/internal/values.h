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
// This file contains the definitions for the WASM bytecode methods that     //
// directly parse values, e.g. integers, floats, vectors, etc.               //
//                                                                           //
// It is included at the bottom of `reader_base.h`, so it shouldn't be       //
// included directly.                                                        //
//                                                                           //
//======---------------------------------------------------------------======//

#pragma once

#include "cobalt/support/clang.h"
#include "cobalt/support/types.h"
#include <bit>
#include <cstddef>
#include <cstdint>

namespace cobalt::internal {

/// Each iteration has the next 7 bits of the integer
inline constexpr u64 bits_per_iteration = 7;

/// We need to mask off these bits for processing
inline constexpr u64 leb128_value = 0x7F;

/// We need to mask off this bit to determine when to end
inline constexpr u64 leb128_continue = 0x80;

/// Same h
inline constexpr u64 leb128_sign = 0x40;

} // namespace cobalt::internal

namespace cobalt {

template <typename Derived> float WasmBytecodeReaderBase<Derived>::read_f32() {
  auto data = consume_n<4>();

  return std::bit_cast<float>(data);
}

template <typename Derived> double WasmBytecodeReaderBase<Derived>::read_f64() {
  auto data = consume_n<8>();

  return std::bit_cast<float>(data);
}

template <typename Derived> u8 WasmBytecodeReaderBase<Derived>::read_byte() {
  return consume();
}

template <typename Derived> u8 WasmBytecodeReaderBase<Derived>::read_u8() {
  return read_leb128_unsigned<8>();
}

template <typename Derived> u16 WasmBytecodeReaderBase<Derived>::read_u16() {
  return read_leb128_unsigned<16>();
}

template <typename Derived> u32 WasmBytecodeReaderBase<Derived>::read_u32() {
  return read_leb128_unsigned<32>();
}

template <typename Derived> u64 WasmBytecodeReaderBase<Derived>::read_u64() {
  return read_leb128_unsigned<64>();
}

template <typename Derived> i8 WasmBytecodeReaderBase<Derived>::read_i8() {
  return read_leb128_signed<8>();
}

template <typename Derived> i16 WasmBytecodeReaderBase<Derived>::read_i16() {
  return read_leb128_signed<16>();
}

template <typename Derived> i32 WasmBytecodeReaderBase<Derived>::read_i32() {
  return read_leb128_signed<32>();
}

template <typename Derived> i64 WasmBytecodeReaderBase<Derived>::read_i64() {
  return read_leb128_signed<64>();
}

template <typename Derived>
template <typename T>
absl::InlinedVector<T, 4> WasmBytecodeReaderBase<Derived>::read_vec(
    T (WasmBytecodeReaderBase<Derived>::*consume_fn)()) {
  auto len = read_u32();
  auto out = std::vector<T>{};
  auto& self = *this;

  out.reserve(len);

  for (auto i = u32{0}; i < len; ++i) {
    out.push_back(self.*consume_fn());
  }

  return out;
}

template <typename Derived> std::string WasmBytecodeReaderBase<Derived>::read_name() {
  // for the sake of avoiding a potential extra allocation, we're reading directly into
  // a `std::string` rather than using the intermediate buffer. This does
  // duplicate the code, but it's worth it
  auto len = read_u32();
  auto out = std::string{};

  out.reserve(len);

  for (auto i = u32{0}; i < len; ++i) {
    out.push_back(static_cast<char>(read_byte()));
  }

  return out;
}

template <typename Derived>
template <int N>
internal::LEB128ReaderResult WasmBytecodeReaderBase<Derived>::read_leb128_internal() {
  static_assert(N <= 64,
      "LEB128 guarantees that the maximum number of bytes to store uN <= ceil(N / 7). We only "
      "account for N <= 64");

  auto result = u64{0};
  auto shift = u64{0};
  u64 last_byte;

  // we need to both have the normal bounds (stop reading when we hit a byte without MSB set)
  // and also check that we aren't reading too many bytes, valid encodings have <= ceil(N / 7) bytes
  //
  // we can do this without storing any additional state by comparing `shift` to `N`, since if we
  // hit our max number of bytes `shift` will go past `N` after we read the last byte
  do {
    last_byte = static_cast<u64>(consume());
    result |= (last_byte & internal::leb128_value) << shift;
    shift += internal::bits_per_iteration;
  } while ((last_byte & internal::leb128_continue) != 0 && shift < N);

  // if we hit our max number of iterations AND the last byte had MSB set, we need to bail.
  // WASM sets a maximum size limit on LEB128 integers, and we need to enforce it
  if ((last_byte & internal::leb128_continue) != 0 && shift >= N) [[unlikely]] {
    throw WasmBytecodeReadException(
        "invalid LEB128 integer, expected end of integer but did not get one");
  }

  return {.result = result, .shift = shift, .last_byte = last_byte};
}

template <typename Derived>
template <int N>
u64 WasmBytecodeReaderBase<Derived>::read_leb128_unsigned() {
  return this->read_leb128_internal<N>().result;
}

template <typename Derived>
template <int N>
i64 WasmBytecodeReaderBase<Derived>::read_leb128_signed() {
  auto parsed = this->read_leb128_internal<N>();
  auto result = parsed.result;

  // the sign bit of byte is second high-order bit (0x40). if that's set,
  // we need to sign-extend our result
  if ((parsed.shift < 64) && (parsed.last_byte & internal::leb128_sign) != 0) {
    result |= (~u64{0} << parsed.shift);
  }

  return std::bit_cast<i64>(result);
}

} // namespace cobalt