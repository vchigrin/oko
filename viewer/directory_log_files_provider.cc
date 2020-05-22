// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/directory_log_files_provider.h"

#include <utility>

namespace oko {

// Lists log files in directory, non-recursively.
DirectoryLogFilesProvider::DirectoryLogFilesProvider(
    std::filesystem::path directory_path) noexcept
    : directory_path_(std::move(directory_path)) {
}

outcome::std_result<std::vector<LogFileInfo>>
    DirectoryLogFilesProvider::GetLogFileInfos() noexcept {
  std::vector<LogFileInfo> result;
  std::error_code ec;
  auto entries = std::filesystem::directory_iterator(directory_path_, ec);
  if (ec) {
    return ec;
  }
  for (auto entry : entries) {
    if (!entry.is_regular_file()) {
      continue;
    }
    std::string file_name = entry.path().filename().native();
    if (CanBeLogFileName(file_name)) {
      result.emplace_back(LogFileInfo{std::move(file_name), entry.file_size()});
    }
  }
  return result;
}

outcome::std_result<std::unique_ptr<LogFile>>
    DirectoryLogFilesProvider::FetchLog(
        const std::string& log_file_name) noexcept {
  return CreateFileForPath(directory_path_ / log_file_name);
}

}  // namespace oko
