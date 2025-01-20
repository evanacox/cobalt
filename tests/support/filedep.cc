//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2025 Evan Cox <evanacox00@gmail.com>. All rights reserved.      //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//

#include "./filedep.h"
#include "absl/container/flat_hash_map.h"
#include "gtest/gtest.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

// invariant: for i,
//
//  names[i] = name of the file, e.g. "leb128_signed.txt"
//  path_without_name[i] = parent path of the file, e.g. "reader/leb128/"
//  path_with_name[i] = path + name, e.g. "reader/leb128/leb128_signed.txt"
//  contents[i] = contents of file
//
std::vector<std::unique_ptr<std::string>> names;
std::vector<std::unique_ptr<std::string>> path_without_name;
std::vector<std::unique_ptr<std::string>> path_with_name;
std::vector<std::unique_ptr<std::string>> contents;

// maps path_with_name => contents
absl::flat_hash_map<std::string_view, std::string_view> path_to_contents;

// maps path_without_name => list of files sharing the same path_without_name
absl::flat_hash_map<std::string_view, std::vector<tests::TestFile>> subdir_to_files;

std::string read_file(const fs::path& path) {
  auto file = std::ifstream{path};
  auto buffer = std::stringstream{};

  buffer << file.rdbuf();

  return std::move(buffer).str();
}

struct ReadTestCaseFiles {
  // abusing static constructor to read all these prior to any test-cases running
  ReadTestCaseFiles() {
    auto current = fs::current_path();
    auto cases_dir = current / "tests" / "deps";

    if (!fs::exists(cases_dir) || !fs::is_directory(cases_dir)) {
      std::cerr << "test executable must be run with cwd as the root of the project, "
                   "file dependencies need to be read from `<root>/tests`\n";

      std::abort();
    }

    for (const auto& entry : fs::recursive_directory_iterator(cases_dir)) {
      if (fs::is_directory(entry)) {
        continue;
      }

      auto rooted_at_cases = fs::relative(entry, cases_dir);
      auto file_content = read_file(entry);
      auto i = names.size();

      names.push_back(std::make_unique<std::string>(rooted_at_cases.filename().generic_string()));
      path_without_name.push_back(
          std::make_unique<std::string>(rooted_at_cases.parent_path().generic_string()));
      path_with_name.push_back(std::make_unique<std::string>(rooted_at_cases.generic_string()));
      contents.push_back(std::make_unique<std::string>(std::move(file_content)));

      path_to_contents.emplace(*path_with_name[i], *contents[i]);

      // create the vector if it doesn't exist yet
      auto [it, _] = subdir_to_files.try_emplace(*path_without_name[i]);

      it->second.push_back(tests::TestFile{
          .name = *names[i],
          .contents = *contents[i],
      });
    }

    (void)0;
  }
} instance;

} // namespace

std::string_view tests::test_file(std::string_view path) {
  auto it = path_to_contents.find(path);

  if (it == path_to_contents.end()) {
    ADD_FAILURE();

    return "";
  }

  return it->second;
}

std::span<tests::TestFile> tests::test_subdir(std::string_view path) {
  auto it = subdir_to_files.find(path);

  if (it == subdir_to_files.end()) {
    ADD_FAILURE();

    return {};
  }

  return it->second;
}
