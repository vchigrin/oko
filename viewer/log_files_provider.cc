// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/log_files_provider.h"

#include "viewer/log_formats/memorylog_log_file.h"
#include "viewer/log_formats/text_log_file.h"

namespace oko {

bool LogFilesProvider::CanBeLogFileName(
    const std::string& file_name) const noexcept {
  return MemorylogLogFile::NameMatches(file_name) ||
      TextLogFile::NameMatches(file_name);
}

}  // namespace oko
