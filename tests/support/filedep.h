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

#include "absl/strings/str_cat.h"
#include "gtest/gtest.h"
#include <charconv>
#include <concepts>
#include <cstdlib>
#include <limits>
#include <span>
#include <string_view>
#include <system_error>

namespace tests {

struct TestFile {
  std::string_view name;
  std::string_view contents;
};

/// Gets the file contents of the test support file at `path`.
///
/// \param path File path rooted at `cases/`
/// \return The contents of the file
std::string_view test_file(std::string_view path);

/// Gets a view over every file in a subdirectory of `cases/`
///
/// \param path Path to the subdirectory
/// \return
std::span<TestFile> test_subdir(std::string_view path);

/// Parses an integer from a string, when the string is expected to contain
/// a valid integer (e.g. from a test file). Aborts if any failure occurs.
///
/// \tparam T The type to parse the value as
/// \param string The string to parse
/// \return The parsed value
template <std::integral T> T parse(std::string_view string, int base = 10) {
  auto value = T{};
  auto [_, ec] = std::from_chars(string.data(), string.data() + string.size(), value, base);

  if (ec != std::errc{}) [[unlikely]] {
    std::abort();
  }

  return value;
}

} // namespace tests