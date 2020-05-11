// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/status_window.h"

#include <utility>

#include "viewer/color_manager.h"
#include "viewer/ncurses_helpers.h"

namespace oko {

StatusWindow::StatusWindow(
    int start_row,
    int start_col,
    int num_columns) noexcept
    : Window(start_row, start_col, kRows, num_columns) {
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
      current_status_.file_name.c_str());
  wclrtoeol(window_.get());
  mvwprintw(
      window_.get(),
      1, 0,
      "Total %lu records.",
      current_status_. total_records);
  const uint64_t marked_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          current_status_.marked_duration).count();
  const uint64_t marked_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          current_status_.marked_duration -
          std::chrono::milliseconds(marked_ms)).count();

  if (current_status_.marked_records > 0) {
    WithColor color(window_, status_mark_color_pair_);
    wprintw(
        window_.get(),
        " Marked %lu records, %llu.%06llu ms marked",
        current_status_.marked_records,
        marked_ms,
        marked_ns);
  }
  wclrtoeol(window_.get());
}

void StatusWindow::HandleKeyPress(int key) noexcept {
}

}  // namespace oko
