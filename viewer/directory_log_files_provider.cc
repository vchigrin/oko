// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/directory_log_files_provider.h"

#include <utility>

#include "viewer/log_formats/memorylog_log_file.h"
#include "viewer/log_formats/text_log_file.h"

namespace oko {

// Lists log files in directory, non-recursively.
DirectoryLogFilesProvider::DirectoryLogFilesProvider(
    std::filesystem::path directory_path) noexcept
    : directory_path_(std::move(directory_path)) {
}

std::vector<LogFileInfo>
    DirectoryLogFilesProvider::GetLogFileInfos() noexcept {
  std::vector<LogFileInfo> result;
  for (auto entry : std::filesystem::directory_iterator(directory_path_)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    std::string file_name = entry.path().filename().native();
    if (MemorylogLogFile::NameMatches(file_name) ||
        TextLogFile::NameMatches(file_name)) {
      result.emplace_back(LogFileInfo{std::move(file_name), entry.file_size()});
    }
  }
  return result;
}

std::filesystem::path DirectoryLogFilesProvider::FetchLog(
    const std::string& log_file_name) noexcept {
  return directory_path_ / log_file_name;
}

}  // namespace oko
