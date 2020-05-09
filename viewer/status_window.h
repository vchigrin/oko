// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <ncurses.h>

#include <memory>
#include <string>

#include "viewer/log_record.h"


namespace oko {

struct StatusInfo {
  uint64_t total_records = 0;
  uint64_t marked_records = 0;
  LogRecord::time_point::duration marked_duration;
};

// Always fixed height status window, usually at the bottom of screen.
class StatusWindow {
 public:
  static constexpr int kRows = 2;

  StatusWindow(
      std::string file_name,
      int start_row,
      int start_col,
      int num_columns) noexcept;

  void UpdateStatus(const StatusInfo& status) noexcept {
    current_status_ = status;
  }

  void Display() noexcept;

 private:
  const std::string file_name_;
  std::unique_ptr<WINDOW, int(*)(WINDOW*)> window_;
  StatusInfo current_status_;
};

}  // namespace oko
