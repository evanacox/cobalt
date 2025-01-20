//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2025 Evan Cox <evanacox00@gmail.com>. All rights reserved.      //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//

#include "./impl.h"
#include "cobalt/bc/reader_base.h"
#include "support/filedep.h"
#include "gtest/gtest.h"
#include <array>
#include <concepts>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

// NOLINTBEGIN(readability-magic-numbers)

TEST(LEB128UnsignedSingle, WasmReaderLEB128) {
  // 33 == 0x22
  auto bytes = std::array<std::uint8_t, 1>{0x22};
  auto reader = tests::WasmReaderTestImpl{bytes};

  ASSERT_EQ(reader.read_leb128_unsigned<64>(), 34);
}

TEST(LEB128UnsignedMultiple, WasmReaderLEB128) {
  // 624485 == [0xe5, 0x8e, 0x26]
  auto bytes = std::array<std::uint8_t, 3>{0xE5, 0x8E, 0x26};
  auto reader = tests::WasmReaderTestImpl{bytes};

  ASSERT_EQ(reader.read_leb128_unsigned<64>(), 624485);
}

TEST(LEB128UnsignedMany, WasmReaderLEB128) {
  // 524622237 == [0x9d, 0xb3, 0x94, 0xfa, 0x01]
  auto bytes = std::array<std::uint8_t, 5>{0x9D, 0xB3, 0x94, 0xFA, 0x01};
  auto reader = tests::WasmReaderTestImpl{bytes};

  ASSERT_EQ(reader.read_leb128_unsigned<64>(), 524622237);
}

TEST(LEB128UnsignedMax, WasmReaderLEB128) {
  // 17278784277645343305 == [0xC9, 0xF4, 0x9E, 0xDD, 0x8E, 0xD8, 0xA4, 0xE5, 0xEF, 0x01]
  auto bytes =
      std::array<std::uint8_t, 10>{0xC9, 0xF4, 0x9E, 0xDD, 0x8E, 0xD8, 0xA4, 0xE5, 0xEF, 0x01};
  auto reader = tests::WasmReaderTestImpl{bytes};

  ASSERT_EQ(reader.read_leb128_unsigned<64>(), 17278784277645343305ul);
}

TEST(LEB128UnsignedTooManyBytesThrows, WasmReaderLEB128) {
  // 11 bytes should fail for 64-bit read
  auto bytes = std::array<std::uint8_t,
      11>{0xC9, 0xF4, 0x9E, 0xDD, 0x8E, 0xD8, 0xA4, 0xE5, 0xEF, 0xEF, 0x01};
  auto reader = tests::WasmReaderTestImpl{bytes};

  ASSERT_THROW(auto _ = reader.read_leb128_unsigned<64>(), fw::WasmBytecodeReadException);
}

// NOLINTEND(readability-magic-numbers)

namespace {

template <std::integral T, std::invocable<T, std::span<std::uint8_t>> Fn>
void parse_from_file(std::string_view file, Fn per_line_test) {
  auto contents = tests::test_file(file);
  auto rest = contents;
  auto next_newline = rest.find('\n');

  for (; !rest.empty(); rest = rest.substr(next_newline + 1), next_newline = rest.find('\n')) {
    if (next_newline == std::string_view::npos) {
      break;
    }

    auto line = rest.substr(0, next_newline);

    // format:
    // EXPECTED HEX_BYTE_PATTERN_OF_LEB128_REPRESENTATION\n
    auto space = line.find(' ');
    auto expected = line.substr(0, space);
    auto byte_pattern = line.substr(space + 1);

    if (byte_pattern.ends_with("\r")) {
      byte_pattern.remove_suffix(1);
    }

    auto expected_value = tests::parse<T>(expected);
    auto leb128_bytes = std::vector<std::uint8_t>{};

    while (!byte_pattern.empty()) {
      // byte_pattern is a hex string, 2 chars => one byte
      leb128_bytes.push_back(tests::parse<std::uint8_t>(byte_pattern.substr(0, 2), 16));
      byte_pattern = byte_pattern.substr(2);
    }

    per_line_test(expected_value, std::span{leb128_bytes});
  }
}

} // namespace

TEST(LEB128UnsignedTestFile, WasmReaderLEB128) {
  // 2**23 - 1
  constexpr auto ieee_single_significand_max = std::uint64_t{8'388'607};

  // 2**52 - 1
  constexpr auto ieee_double_significand_max = std::uint64_t{4'503'599'627'370'495};

  // NOLINTNEXTLINE(readability-function-cognitive-complexity)
  auto per_line_test = [](std::uint64_t value, std::span<std::uint8_t> bytes) {
    if (value < std::numeric_limits<std::uint8_t>::max()) {
      auto reader = tests::WasmReaderTestImpl{bytes};

      EXPECT_EQ(reader.read_leb128_unsigned<8>(), value);
    }

    if (value < std::numeric_limits<std::uint16_t>::max()) {
      auto reader = tests::WasmReaderTestImpl{bytes};

      EXPECT_EQ(reader.read_leb128_unsigned<16>(), value);
    }

    if (value < ieee_single_significand_max) {
      auto reader = tests::WasmReaderTestImpl{bytes};

      EXPECT_EQ(reader.read_leb128_unsigned<23>(), value);
    }

    if (value < std::numeric_limits<std::uint32_t>::max()) {
      auto reader = tests::WasmReaderTestImpl{bytes};

      EXPECT_EQ(reader.read_leb128_unsigned<32>(), value);
    }

    if (value < ieee_double_significand_max) {
      auto reader = tests::WasmReaderTestImpl{bytes};

      EXPECT_EQ(reader.read_leb128_unsigned<52>(), value);
    }

    if (value < std::numeric_limits<std::uint64_t>::max()) {
      auto reader = tests::WasmReaderTestImpl{bytes};

      EXPECT_EQ(reader.read_leb128_unsigned<64>(), value);
    }
  };

  parse_from_file<std::uint64_t>("bc/leb128/unsigned.txt", per_line_test);
}

namespace {

template <typename T> bool within_bounds_of(std::int64_t value) noexcept {
  return std::numeric_limits<T>::min() <= value && value <= std::numeric_limits<T>::max();
}

} // namespace

TEST(LEB128SignedTestFile, WasmReaderLEB128) {
  // NOLINTNEXTLINE(readability-function-cognitive-complexity)
  auto per_line_test = [](std::int64_t value, std::span<std::uint8_t> bytes) {
    if (within_bounds_of<std::int8_t>(value)) {
      auto reader = tests::WasmReaderTestImpl{bytes};

      EXPECT_EQ(reader.read_leb128_signed<8>(), value);
    }

    if (within_bounds_of<std::int16_t>(value)) {
      auto reader = tests::WasmReaderTestImpl{bytes};

      EXPECT_EQ(reader.read_leb128_signed<16>(), value);
    }

    if (within_bounds_of<std::int32_t>(value)) {
      auto reader = tests::WasmReaderTestImpl{bytes};

      EXPECT_EQ(reader.read_leb128_signed<32>(), value);
    }

    if (within_bounds_of<std::int64_t>(value)) {
      auto reader = tests::WasmReaderTestImpl{bytes};

      EXPECT_EQ(reader.read_leb128_signed<64>(), value);
    }
  };

  parse_from_file<std::int64_t>("bc/leb128/signed.txt", per_line_test);
}
