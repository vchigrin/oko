// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <boost/range/iterator_range.hpp>
#include <string>
#include <vector>

#include "viewer/log_file_impl.h"

namespace oko {

// Class for parsing dump files produced by memorylog library,
// see https://github.com/agrianius/memorylog
class MemorylogLogFile : public LogFileImpl {
 public:
  static bool NameMatches(const std::string& file_name) noexcept;

 private:
  std::error_code ParseImpl(
      std::string_view file_data,
      std::vector<LogRecord>* records) noexcept override;

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
      std::vector<LogRecord>* records,
      RawRecordsRange added_region,
      const uint64_t first_raw_time_stamp,
      const LogRecord::time_point first_time_point,
      const uint64_t second_raw_time_stamp,
      const LogRecord::time_point second_time_point);
};

}  // namespace oko
