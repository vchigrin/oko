// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/status_window.h"

#include <algorithm>
#include <boost/format.hpp>
#include <utility>

#include "viewer/ui/color_manager.h"
#include "viewer/ui/ncurses_helpers.h"

namespace oko {

StatusWindow::StatusWindow(
    AppModel* model,
    int start_row,
    int start_col,
    int num_columns) noexcept
    : Window(start_row, start_col, kRows, num_columns),
      app_model_(model) {
  ColorManager& cm = ColorManager::instance();
  status_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_WHITE);
  status_mark_color_pair_ = cm.RegisterColorPair(COLOR_RED, COLOR_WHITE);
}

void StatusWindow::DisplayImpl() noexcept {
  wbkgdset(window_.get(), COLOR_PAIR(status_color_pair_));
  const int max_x = getmaxx(window_.get());

  // Right-justify total records text.
  const std::string total_records_text = boost::str(
      boost::format(" %1% records") % app_model_->unfiltered_records_count());
  const int total_records_x = std::max<int>(
      0, max_x - total_records_text.size());

  if (total_records_x > 0) {
    auto file_paths = app_model_->GetFilePaths();
    if (file_paths.size() == 1) {
      std::string file_name = file_paths[0];
      const char* displayed_part = file_name.c_str();
      if (total_records_x < file_name.size()) {
        // End patf of file name is more interesting.
        displayed_part += (file_name.size() - total_records_x);
      }
      mvwaddstr(
          window_.get(),
          0, 0,
          displayed_part);
    } else {
      mvwprintw(
          window_.get(),
          0, 0,
          "%zi files",
          file_paths.size());
    }
  }

  wclrtoeol(window_.get());
  mvwaddstr(
      window_.get(),
      0, total_records_x,
      total_records_text.c_str());

  const auto marked_records_count = app_model_->marked_records_count();
  if (marked_records_count > 0) {
    const auto marked_duration = app_model_->marked_duration();
    const uint64_t marked_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            marked_duration).count();
    const uint64_t marked_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            marked_duration -
            std::chrono::milliseconds(marked_ms)).count();
    WithColor color(window_, status_mark_color_pair_);
    mvwprintw(
        window_.get(),
        1, 0,
        "Marked %zi records, %llu.%06llu ms",
        marked_records_count,
        marked_ms,
        marked_ns);
  } else {
    wmove(window_.get(), 1, 0);
  }
  wclrtoeol(window_.get());

  // Right-justify current position text.
  const std::string cur_pos_text = boost::str(boost::format(
      "Record %1%/%2%") %
      app_model_->selected_record() %
      app_model_->filtered_records_count());
  const int cur_pos_x = std::max<int>(0, max_x - cur_pos_text.length());
  mvwaddstr(
      window_.get(),
      1, cur_pos_x,
      cur_pos_text.c_str());
}

void StatusWindow::HandleKeyPress(int key) noexcept {
}

}  // namespace oko
