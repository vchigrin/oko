// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/status_window.h"

#include <utility>

#include "viewer/ncurses_helpers.h"

namespace oko {

StatusWindow::StatusWindow(
    std::string file_name,
    int start_row,
    int start_col,
    int num_columns) noexcept
    : Window(start_row, start_col, kRows, num_columns),
      file_name_(std::move(file_name)) {
  wbkgd(window_.get(), COLOR_PAIR(kStatusColorPair));
}

void StatusWindow::DisplayImpl() noexcept {
  mvwprintw(
      window_.get(),
      0, 0,
      "File %s\nTotal %lu records.",
      file_name_.c_str(),
      current_status_. total_records);
  const uint64_t marked_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          current_status_.marked_duration).count();
  const uint64_t marked_ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          current_status_.marked_duration -
          std::chrono::milliseconds(marked_ms)).count();

  if (current_status_.marked_records > 0) {
    WithColor color(window_, kStatusMarkColorPair);
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
