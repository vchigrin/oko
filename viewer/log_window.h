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
      int num_rows, int num_columns) noexcept;
  void Display() noexcept;
  void HandleKeyPress(int key) noexcept;

  uint64_t total_records() const noexcept {
    return view_->GetRecords().size();
  }

  uint64_t marked_records() const noexcept {
    return marked_records_end_ - marked_records_begin_;
  }

  LogRecord::time_point::duration marked_duration() const noexcept {
    if (marked_records_end_ == marked_records_begin_) {
      return {};
    }
    const auto& records = view_->GetRecords();
    return records[marked_records_end_ - 1].timestamp() -
        records[marked_records_begin_].timestamp();
  }

 private:
  size_t GetDisplayedRecordAfterLast() const noexcept;
  void DisplayMessage(int row, const std::string_view& message) noexcept;
  void DisplayLevel(bool is_marked,
      int row, const LogLevel level) noexcept;
  void DisplayTime(bool is_marked,
      int row, const LogRecord::time_point time_point) noexcept;
  void MaybeExtendMarking() noexcept;
  bool IsMarked(size_t index) const noexcept {
    return index >= marked_records_begin_ && index < marked_records_end_;
  }
  size_t GetRecordUnderCursor() const noexcept {
    return first_shown_record_ + cursor_line_;
  }

  std::unique_ptr<LogView> view_;
  std::unique_ptr<WINDOW, int(*)(WINDOW*)> window_;
  int start_row_;
  int start_col_;
  int num_rows_;
  int num_columns_;
  size_t first_shown_record_ = 0;
  size_t message_horz_offset_ = 0;
  int cursor_line_ = 0;
  // Record where marking was started - acts as anchor for updating
  // marked region.
  size_t marked_anchor_record_ = 0;
  // Index of first marked record, inclusively.
  size_t marked_records_begin_ = 0;
  // One plus index of last marked record.
  size_t marked_records_end_ = 0;
  // Whether we're marking region during movement.
  bool marking_ = false;
};

}  // namespace oko
