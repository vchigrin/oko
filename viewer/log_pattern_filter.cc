// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/log_pattern_filter.h"

namespace oko {

LogPatternFilter::LogPatternFilter(
    const LogView* parent_view,
    const std::string& pattern,
    bool is_include_filter)
    : pattern_(pattern),
      is_include_filter_(is_include_filter) {
  const std::vector<LogRecord>& parent_records = parent_view->GetRecords();
  filtered_records_.reserve(parent_records.size());
  for (const LogRecord& rec : parent_records) {
    const bool has_pattern =
        rec.message().find(pattern.c_str()) != std::string_view::npos;
    if (has_pattern == is_include_filter) {
      filtered_records_.emplace_back(rec);
    } else {
      ++filtered_out_records_count_;
    }
  }
}

const std::vector<LogRecord>& LogPatternFilter::GetRecords() const noexcept {
  return filtered_records_;
}

}  // namespace oko
