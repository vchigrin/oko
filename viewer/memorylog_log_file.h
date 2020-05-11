// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/range/iterator_range.hpp>
#include <string>
#include <vector>

#include "viewer/log_file.h"

namespace oko {

// Class for parsing dump files produced by memorylog library,
// see https://github.com/agrianius/memorylog
class MemorylogLogFile : public LogFile {
 public:
  bool Parse(const std::string& file_path) noexcept override;
  const std::vector<LogRecord>& GetRecords() const noexcept override {
    return records_;
  }
  const std::string& file_path() const noexcept override {
    return file_path_;
  }

 private:
  struct RawRecord {
    uint64_t raw_timestamp;
    std::string_view message;
  };
  bool FillRecord(std::string_view entry_data, RawRecord* record) noexcept;
  bool ExtractTimestampFromRecord(
      const RawRecord&,
      LogRecord::time_point* result_timestamp) noexcept;
  using RawRecordsRange =
      boost::iterator_range<std::vector<RawRecord>::const_iterator>;
  // Adds records from range |added_region|,
  // using two anchor time points data.
  // first_raw_time_stamp must be strictly less then second_raw_time_stamp.
  // Assumes that all records from |added_region| are ordered by raw_timestamp.
  // First anchor time point must not lie in the middle of added region.
  void ProcessRecords(
      RawRecordsRange added_region,
      const uint64_t first_raw_time_stamp,
      const LogRecord::time_point first_time_point,
      const uint64_t second_raw_time_stamp,
      const LogRecord::time_point second_time_point);

  std::vector<LogRecord> records_;
  boost::iostreams::mapped_file_source mapped_file_;
  std::string file_path_;
};

}  // namespace oko
