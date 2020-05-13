// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once

#include <vector>

#include "viewer/app_model.h"
#include "viewer/log_view.h"
#include "viewer/window.h"

namespace oko {

class LogWindow : public Window {
 public:
  LogWindow(
      AppModel* model,
      int start_row,
      int start_col,
      int num_rows,
      int num_columns) noexcept;
  void HandleKeyPress(int key) noexcept override;

 private:
  void DisplayImpl() noexcept override;
  size_t GetDisplayedRecordAfterLast() const noexcept;
  void DisplayMessage(int row, const std::string_view& message) noexcept;
  void DisplayLevel(bool is_marked,
      int row, const LogLevel level) noexcept;
  void DisplayTime(bool is_marked,
      int row, const LogRecord::time_point time_point) noexcept;
  void MaybeExtendMarking() noexcept;
  void FilterSetChanged(const std::vector<LogPatternFilter*>&) noexcept;
  void SelectedRecordChanged(size_t selected_record) noexcept;

  const LogView* view_;
  AppModel* app_model_;
  boost::signals2::connection filter_set_changed_conn_;
  boost::signals2::connection selected_record_changed_conn_;
  size_t first_shown_record_ = 0;
  size_t message_horz_offset_ = 0;
  // Record where marking was started - acts as anchor for updating
  // marked region.
  size_t marked_anchor_record_ = 0;
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
