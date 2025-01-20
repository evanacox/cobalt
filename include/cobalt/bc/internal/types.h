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

namespace cobalt {

template <typename Derived> ValueType WasmBytecodeReaderBase<Derived>::read_val_ty() {
  auto byte = read_byte();

  switch (byte) {
    case 0x6F: return ValueType::extern_ref;
    case 0x70: return ValueType::func_ref;
    case 0x7B: return ValueType::v128;
    case 0x7C: return ValueType::f64;
    case 0x7D: return ValueType::f32;
    case 0x7E: return ValueType::i64;
    case 0x7F: return ValueType::i32;
    default: {
      [[unlikely]];
      throw WasmBytecodeReadException(
          absl::StrCat("unknown type identifier '", absl::Hex(byte), "'"));
    }
  }
}

template <typename Derived>
absl::InlinedVector<ValueType, 4> WasmBytecodeReaderBase<Derived>::read_result_ty() {
  return read_vec(read_val_ty());
}

template <typename Derived> FunctionType WasmBytecodeReaderBase<Derived>::read_function_ty() {
  expect(0x60);
}

template <typename Derived> Limit WasmBytecodeReaderBase<Derived>::read_limit() {
  //
}

template <typename Derived> MemoryType WasmBytecodeReaderBase<Derived>::read_memory_ty() {
  //
}

template <typename Derived> TableType WasmBytecodeReaderBase<Derived>::read_table_ty() {
  //
}

template <typename Derived> GlobalType WasmBytecodeReaderBase<Derived>::read_global_ty() {
  //
}

} // namespace cobalt