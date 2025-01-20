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

#include <type_traits>

namespace cobalt {

/// Concept wrapper for `std::is_trivially_destructible`
///
/// \tparam T The type that must be trivially destructible
template <typename T>
concept TriviallyDestructible = std::is_trivially_destructible_v<T>;

} // namespace cobalt