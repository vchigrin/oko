// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/log_files_provider.h"

#include <utility>

#include "viewer/log_formats/memorylog_log_file.h"
#include "viewer/log_formats/text_log_file.h"

namespace oko {

bool LogFilesProvider::CanBeLogFileName(
    const std::string& file_name) const noexcept {
  return MemorylogLogFile::NameMatches(file_name) ||
      TextLogFile::NameMatches(file_name);
}

std::unique_ptr<LogFile> LogFilesProvider::CreateFileForPath(
    std::filesystem::path file_path) const noexcept {
  if (oko::TextLogFile::NameMatches(file_path.filename())) {
    return std::make_unique<oko::TextLogFile>(std::move(file_path));
  } else if (oko::MemorylogLogFile::NameMatches(file_path.filename())) {
    return std::make_unique<oko::MemorylogLogFile>(std::move(file_path));
  } else {
    assert(false);
    std::abort();
  }
}

}  // namespace oko
