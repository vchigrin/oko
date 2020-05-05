// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <ncurses.h>

#include <memory>

#include "viewer/log_view.h"

namespace oko {

class LogWindow {
 public:
  LogWindow(
      std::unique_ptr<LogView> view,
      int start_row,
      int start_col,
      int num_rows,
      int num_columns) noexcept;
  void Move(
      int start_row, int start_col,
      int num_rows_, int num_columns) noexcept;
  void Display() noexcept;
  void HandleKeyPress(int key) noexcept;

 private:
  size_t GetDisplayedRecordAfterLast() const noexcept;
  void DisplayMessage(int row, const std::string_view& message) noexcept;
  void DisplayLevel(int row, const LogLevel level) noexcept;
  void DisplayTime(int row, const LogRecord::time_point time_point) noexcept;

  std::unique_ptr<LogView> view_;
  std::unique_ptr<WINDOW, int(*)(WINDOW*)> window_;
  int start_row_;
  int start_col_;
  int num_rows_;
  int num_columns_;
  size_t first_shown_record_ = 0;
  size_t message_horz_offset_ = 0;
};

}  // namespace oko
