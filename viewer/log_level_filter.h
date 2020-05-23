// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <unordered_set>
#include <vector>

#include "viewer/log_filter.h"

namespace oko {

// Filters records based on logging levels.
class LogLevelFilter : public LogFilter {
 public:
  LogLevelFilter(
      const LogView* parent_view,
      std::unordered_set<LogLevel> levels_to_include);

  const std::vector<LogRecord>& GetRecords() const noexcept override {
    return filtered_records_;
  }

  size_t filtered_out_records_count() const noexcept override {
    return filtered_records_count_;
  }

  const std::unordered_set<LogLevel>& levels_to_include() const noexcept {
    return levels_to_include_;
  }

  Type type() const noexcept override {
    return Type::kLevel;
  }

 private:
  std::vector<LogRecord> filtered_records_;
  const std::unordered_set<LogLevel> levels_to_include_;
  size_t filtered_records_count_ = 0;
};

}  // namespace oko
