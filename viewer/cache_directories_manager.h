// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <boost/outcome/result.hpp>
#include <filesystem>
#include <string>

namespace oko {

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

// Creates unique cache directories based on provided argument.
class CacheDirectoriesManager {
 public:
  CacheDirectoriesManager() noexcept;
  outcome::std_result<std::filesystem::path> DirectoryForS3Url(
      const std::string& s3_directory_url) noexcept;

  outcome::std_result<std::filesystem::path> DirectoryForFile(
      const std::filesystem::path& file_path) noexcept;

  outcome::std_result<std::filesystem::path> DirectoryForData(
      std::string_view data) noexcept;

  bool is_initialized() const noexcept {
    return !cache_root_path_.empty();
  }

 private:
  outcome::std_result<std::filesystem::path> DirectoryForHash(
      std::string hash) noexcept;

  std::filesystem::path cache_root_path_;
};

}  // namespace oko
