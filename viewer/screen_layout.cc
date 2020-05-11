// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/screen_layout.h"

#include <algorithm>

namespace oko {

ScreenLayout::ScreenLayout(AppModel* model) noexcept
    : app_model_(model),
      log_window_(model->active_view(), 0, 0, 1, 1),
      filter_list_window_(0, 0, 1),
      status_window_(0, 0, 1) {
  filter_set_changed_conn_ =
      app_model_->ConnectFilterSetChanged(
          std::bind(
              &ScreenLayout::FilterSetChanged,
              this,
              std::placeholders::_1));
  RecalcPositions();
}

void ScreenLayout::Display() noexcept {
  log_window_.Display();
  status_window_.Display();
  filter_list_window_.Display();
}

void ScreenLayout::RecalcPositions() noexcept {
  int max_row = 0, max_col = 0;
  getmaxyx(stdscr, max_row, max_col);
  const int filter_window_height = filter_list_window_.GetDesiredHeight();
  filter_list_window_.Move(0, 0, filter_window_height, max_col);
  log_window_.Move(
      filter_window_height, 0,
      std::max(1, max_row - StatusWindow::kRows - filter_window_height),
      max_col);
  status_window_.Move(
      std::max(0, max_row - oko::StatusWindow::kRows), 0,
      StatusWindow::kRows, max_col);
}

void ScreenLayout::HandleKeyPress(int key) noexcept {
  log_window_.HandleKeyPress(key);
}

void ScreenLayout::FilterSetChanged(
    const std::vector<LogPatternFilter*>& active_filters) {
  filter_list_window_.UpdateActiveFilters(active_filters);
  log_window_.SetView(app_model_->active_view());
  RecalcPositions();
}

}  // namespace oko
