// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <boost/iostreams/device/mapped_file.hpp>
#include <optional>
#include <string>
#include <vector>

#include "viewer/log_file.h"

namespace oko {

// Class for parsing log files in some proprientary project.
class TextLogFile : public LogFile {
 public:
  bool Parse(const std::filesystem::path& file_path) noexcept override;
  const std::vector<LogRecord>& GetRecords() const noexcept override {
    return records_;
  }
  const std::filesystem::path& file_path() const noexcept override {
    return file_path_;
  }

  static bool NameMatches(const std::string& file_name) noexcept;

 private:
  struct RawRecordInfo {
    uint64_t sec;
    uint64_t msec;
    uint64_t nsec_counter;
    LogLevel level;
    std::string_view message;
  };
  bool ParseLine(std::string_view line, RawRecordInfo& info) noexcept;
  void AddRecord(const RawRecordInfo& info) noexcept;

  std::vector<LogRecord> records_;
  boost::iostreams::mapped_file_source mapped_file_;
  std::filesystem::path file_path_;
  std::optional<LogRecord::time_point> nsec_counter_base_;
};

}  // namespace oko
