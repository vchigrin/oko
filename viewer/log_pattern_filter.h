// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <vector>
#include <string>
#include "viewer/log_view.h"

namespace oko {

// Filters records based on pattern.
class LogPatternFilter : public LogView {
 public:
  LogPatternFilter(
      const LogView* parent_view,
      const std::string& pattern,
      bool is_include_filter);
  const std::vector<LogRecord>& GetRecords() const noexcept override;
  size_t filtered_out_records_count() const noexcept {
    return filtered_records_count_;
  }

  bool is_include_filter() const noexcept {
    return is_include_filter_;
  }

  const std::string& pattern() const noexcept {
    return pattern_;
  }

 private:
  std::vector<LogRecord> filtered_records_;
  const std::string pattern_;
  const bool is_include_filter_;
  size_t filtered_records_count_ = 0;
};

}  // namespace oko
