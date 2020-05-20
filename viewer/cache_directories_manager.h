// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <filesystem>
#include <string>

namespace oko {

// Creates unique cache directories based on provided argument.
class CacheDirectoriesManager {
 public:
  CacheDirectoriesManager() noexcept;
  std::filesystem::path DirectoryForS3Url(
      const std::string& s3_directory_url) noexcept;

  bool is_initialized() const noexcept {
    return !cache_root_path_.empty();
  }

 private:
  std::filesystem::path cache_root_path_;
};

}  // namespace oko
