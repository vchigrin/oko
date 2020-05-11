// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <string>

#include "viewer/log_record.h"
#include "viewer/window.h"


namespace oko {

struct StatusInfo {
  uint64_t total_records = 0;
  uint64_t marked_records = 0;
  LogRecord::time_point::duration marked_duration;
};

// Always fixed height status window, usually at the bottom of screen.
class StatusWindow : public Window {
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

  void HandleKeyPress(int key) noexcept override;

 private:
  void DisplayImpl() noexcept override;

  const std::string file_name_;
  StatusInfo current_status_;
  int status_color_pair_ = 0;
  int status_mark_color_pair_ = 0;
};

}  // namespace oko
