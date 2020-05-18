// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <boost/iostreams/device/mapped_file.hpp>
#include <filesystem>
#include <string_view>
#include <vector>

#include "viewer/log_file.h"

namespace oko {

// Implementation of LogFile interface.
// Maps whole file into memory during parsing.
class LogFileImpl : public LogFile {
 public:
  // May create inside memory view of the file, so it is expected
  // that file will not be changed or deleted during lifetime of this object.
  bool Parse(const std::filesystem::path& file_path) noexcept override;
  const std::filesystem::path& file_path() const noexcept override;

  const std::vector<LogRecord>& GetRecords() const noexcept override;

 protected:
  virtual void ParseImpl(
      std::string_view file_data,
      std::vector<LogRecord>* records) noexcept = 0;

 private:
  std::vector<LogRecord> records_;
  boost::iostreams::mapped_file_source mapped_file_;
  std::filesystem::path file_path_;
};

}  // namespace oko
