// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/log_level_filter.h"

#include <utility>

namespace oko {

LogLevelFilter::LogLevelFilter(
    const LogView* parent_view,
    std::unordered_set<LogLevel> levels_to_include)
  : levels_to_include_(std::move(levels_to_include)) {
  const std::vector<LogRecord>& parent_records = parent_view->GetRecords();
  filtered_records_.reserve(parent_records.size());
  for (const LogRecord& rec : parent_records) {
    if (levels_to_include_.count(rec.log_level()) != 0) {
      filtered_records_.emplace_back(rec);
    } else {
      ++filtered_records_count_;
    }
  }
}

}  // namespace oko
