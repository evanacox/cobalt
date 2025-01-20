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

#include <string>
#include <string_view>
#include <utility>

namespace cobalt {

/// The base exception type for all `cobalt` failures
class BaseException {
public:
  /// An explanation of the failure.
  ///
  /// \return The explanatory message
  [[nodiscard]] std::string_view what() const {
    return message_;
  }

protected:
  explicit BaseException(std::string_view message) : message_{message} {}

  explicit BaseException(std::string content)
      : optional_content_{std::move(content)},
        message_{optional_content_} {}

private:
  std::string optional_content_;
  std::string_view message_;
};

/// Thrown when a WASM module fails some validation step
///
/// The error message contains
class ValidationFailure : public BaseException {
protected:
  explicit ValidationFailure(std::string failure) : BaseException{std::move(failure)} {}

  explicit ValidationFailure(std::string_view failure) : BaseException{failure} {}
};

} // namespace cobalt