// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <boost/outcome/outcome.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "viewer/cache_directories_manager.h"
#include "viewer/file_decompressor.h"
#include "viewer/log_file.h"

namespace oko {

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

struct LogFileInfo {
  std::string name;
  uint64_t size;
};

// Base class for classes, offering retrieving list of log file names
// and fetching their content.
class LogFilesProvider {
 public:
  explicit LogFilesProvider(
      std::unique_ptr<CacheDirectoriesManager> cache_manager) noexcept;
  virtual ~LogFilesProvider() = default;
  virtual outcome::std_result<std::vector<LogFileInfo>>
      GetLogFileInfos() noexcept = 0;
  // Returns not parsed LogFile instance.
  // |log_file_name| is a name from list, returned by |GetLogFileNames|.
  virtual outcome::std_result<std::unique_ptr<LogFile>> FetchLog(
      const std::string& log_file_name) noexcept = 0;

 protected:
  bool CanBeLogFileName(const std::string& file_name) const noexcept;
  outcome::std_result<std::unique_ptr<LogFile>> CreateFileForPath(
      std::filesystem::path file_path) const noexcept;
  std::vector<std::unique_ptr<FileDecompressor>> decompressors_;
  std::unique_ptr<CacheDirectoriesManager> cache_manager_;
};

}  // namespace oko
