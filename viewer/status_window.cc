// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/status_window.h"

#include <utility>

#include "viewer/color_manager.h"
#include "viewer/ncurses_helpers.h"

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
  mvwprintw(
      window_.get(),
      0, 0,
      "File %s",
      app_model_->file_path().c_str());
  wclrtoeol(window_.get());
  mvwprintw(
      window_.get(),
      1, 0,
      "Total %lu records.",
      app_model_->total_records_count());

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
    wprintw(
        window_.get(),
        " Marked %lu records, %llu.%06llu ms marked",
        marked_records_count,
        marked_ms,
        marked_ns);
  }
  wclrtoeol(window_.get());
}

void StatusWindow::HandleKeyPress(int key) noexcept {
}

}  // namespace oko
