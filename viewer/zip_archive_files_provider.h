// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once

#include <zip.h>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "viewer/log_files_provider.h"

namespace oko {

// Lists log files in .zip file, recursively.
class ZipArchiveFilesProvider : public LogFilesProvider {
 public:
  ZipArchiveFilesProvider(
      std::unique_ptr<CacheDirectoriesManager> cache_manager,
      std::filesystem::path zip_file_path) noexcept;

  outcome::std_result<std::vector<LogFileInfo>>
      GetLogFileInfos() noexcept override;

  outcome::std_result<std::unique_ptr<LogFile>> FetchLog(
      const std::string& log_file_name) noexcept override;

 private:
  outcome::std_result<std::vector<char>>
      ReadCompressedData(zip_uint64_t entry_index) noexcept;
  std::unique_ptr<zip_t, int(*)(zip_t*)> zip_file_;
  std::unordered_map<std::string, zip_uint64_t> name_to_index_;
};

}  // namespace oko
