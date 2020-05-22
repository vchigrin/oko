// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "viewer/log_files_provider.h"

namespace oko {

// Lists log files in directory, non-recursively.
class DirectoryLogFilesProvider : public LogFilesProvider {
 public:
  explicit DirectoryLogFilesProvider(
      std::filesystem::path directory_path) noexcept;
  outcome::std_result<std::vector<LogFileInfo>>
      GetLogFileInfos() noexcept override;

  outcome::std_result<std::unique_ptr<LogFile>> FetchLog(
      const std::string& log_file_name) noexcept override;

 private:
  const std::filesystem::path directory_path_;
};

}  // namespace oko
