// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
// Macro provided by curses.h.
#pragma push_macro("OK")
#undef OK
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#pragma pop_macro("OK")
#include <memory>
#include <string>
#include <vector>

#include "viewer/log_files_provider.h"

namespace oko {

// Lists log files in AWS bucket directory, non-recursively.
class S3LogFilesProvider : public LogFilesProvider {
 public:
  // |cache_directory_path| must exist and must be dedicated to
  // exactly this |directory_url|
  S3LogFilesProvider(
      std::filesystem::path cache_directory_path,
      std::string s3_directory_url) noexcept;
  ~S3LogFilesProvider();
  std::vector<LogFileInfo> GetLogFileInfos() noexcept override;
  std::filesystem::path FetchLog(
      const std::string& log_file_name) noexcept override;

 private:
  void EnsureInitialized() noexcept;
  const std::filesystem::path cache_directory_path_;
  std::string bucket_name_;
  std::string s3_directory_name_;
  Aws::SDKOptions aws_options_;
  std::unique_ptr<Aws::S3::S3Client> s3_client_;
};

}  // namespace oko
