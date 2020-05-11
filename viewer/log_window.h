// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once

#include "viewer/log_view.h"
#include "viewer/window.h"

namespace oko {

class LogWindow : public Window {
 public:
  LogWindow(
      LogView* view,
      int start_row,
      int start_col,
      int num_rows,
      int num_columns) noexcept;
  void HandleKeyPress(int key) noexcept override;

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

  void SetView(LogView* view) noexcept;

 private:
  void DisplayImpl() noexcept override;
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

  LogView* view_;
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
  int time_color_pair_ = 0;
  int debug_color_pair_ = 0;
  int info_color_pair_ = 0;
  int warn_color_pair_ = 0;
  int err_color_pair_ = 0;
  int mark_color_pair_ = 0;
};

}  // namespace oko
