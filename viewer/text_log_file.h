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
  bool Parse(const std::string& file_path) noexcept override;
  const std::vector<LogRecord>& GetRecords() const noexcept override {
    return records_;
  }
  const std::string& file_path() const noexcept override {
    return file_path_;
  }

 private:
  void ParseLine(std::string_view line) noexcept;
  void AddRecord(
      uint64_t sec, uint64_t msec,
      uint64_t nsec_counter,
      std::string_view level_string,
      std::string_view message) noexcept;

  std::vector<LogRecord> records_;
  boost::iostreams::mapped_file_source mapped_file_;
  std::string file_path_;
  std::optional<LogRecord::time_point> nsec_counter_base_;
};

}  // namespace oko
