// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <boost/outcome/outcome.hpp>
#include <filesystem>
#include <string>
#include <vector>

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
  virtual ~LogFilesProvider() = default;
  virtual outcome::std_result<std::vector<LogFileInfo>>
      GetLogFileInfos() noexcept = 0;
  // Returns path to local files with log contents.
  // |log_file_name| is a name from list, returned by |GetLogFileNames|.
  virtual outcome::std_result<std::filesystem::path> FetchLog(
      const std::string& log_file_name) noexcept = 0;

 protected:
  bool CanBeLogFileName(const std::string& file_name) const noexcept;
};

}  // namespace oko
