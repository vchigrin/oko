// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/filters_list_window.h"

#include "viewer/color_manager.h"
#include "viewer/ncurses_helpers.h"

namespace oko {

FilterListWindow::FilterListWindow(
    AppModel* model,
    int start_row,
    int start_col,
    int num_columns) noexcept
    : Window(start_row, start_col, 1, num_columns),
      app_model_(model) {
  ColorManager& cm = ColorManager::instance();
  include_filter_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_GREEN);
  exclude_filter_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_RED);

  filter_set_changed_conn_ =
      app_model_->ConnectFilterSetChanged(
          std::bind(
              &FilterListWindow::FilterSetChanged,
              this,
              std::placeholders::_1));
}

int FilterListWindow::GetDesiredHeight() noexcept {
  return active_filters_.size() + 1;
}

void FilterListWindow::HandleKeyPress(int key) noexcept {
  // TODO(vchigrin): Implement selecting and editing filters.
}

void FilterListWindow::DisplayImpl() noexcept {
  int cur_row = 0;
  int old_bkgd = getbkgd(window_.get());
  for (LogPatternFilter* filter : active_filters_) {
    wbkgdset(window_.get(),
        filter->is_include_filter() ?
            COLOR_PAIR(include_filter_color_pair_) :
            COLOR_PAIR(exclude_filter_color_pair_));
    mvwaddstr(window_.get(), cur_row, 0,
        filter->pattern().c_str());
    wclrtoeol(window_.get());
    ++cur_row;
  }
  wbkgdset(window_.get(), old_bkgd);
  mvwhline(window_.get(), cur_row, 0, 0, num_columns_);
}

void FilterListWindow::FilterSetChanged(
    const std::vector<LogPatternFilter*>& active_filters) noexcept {
  active_filters_ = active_filters;
}

}  // namespace oko
