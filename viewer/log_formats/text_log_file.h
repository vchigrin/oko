// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <optional>
#include <string>
#include <vector>

#include "viewer/log_file_impl.h"

namespace oko {

// Class for parsing log files in some proprientary project.
class TextLogFile : public LogFileImpl {
 public:
  static bool NameMatches(const std::string& file_name) noexcept;

 private:
  std::error_code ParseImpl(
      std::string_view file_data,
      std::vector<LogRecord>* records) noexcept override;

  struct RawRecordInfo {
    uint64_t sec;
    uint64_t msec;
    uint64_t nsec_counter;
    LogLevel level;
    std::string_view message;
  };
  bool ParseLine(std::string_view line, RawRecordInfo& info) noexcept;
  void AddRecord(
      std::vector<LogRecord>* records,
      const RawRecordInfo& info) noexcept;

  std::optional<LogRecord::time_point> nsec_counter_base_;
};

}  // namespace oko
