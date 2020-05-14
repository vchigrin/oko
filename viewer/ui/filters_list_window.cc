// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/filters_list_window.h"

#include <algorithm>

#include "viewer/ui/color_manager.h"
#include "viewer/ui/ncurses_helpers.h"

namespace oko {

namespace {
const int kFilteredOutColumnSize = 16;
const int kTypeColumnSize = 8;
}  // namespace

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
  return active_filters_.empty() ? 0 : active_filters_.size() + 2;
}

void FilterListWindow::HandleKeyPress(int key) noexcept {
  // TODO(vchigrin): Implement selecting and editing filters.
}

void FilterListWindow::DisplayImpl() noexcept {
  DisplayBorders();
  int cur_row = 1;
  int old_bkgd = getbkgd(window_.get());
  const int max_x = getmaxx(window_.get());
  const int max_pattern_len = std::max(
      0, max_x - kFilteredOutColumnSize - kTypeColumnSize);
  const int filtered_out_start = max_x - kFilteredOutColumnSize;

  static const std::string_view kIncludeType = "Incl:";
  static const std::string_view kExcludeType = "Excl:";
  for (LogPatternFilter* filter : active_filters_) {
    wbkgdset(window_.get(),
        filter->is_include_filter() ?
            COLOR_PAIR(include_filter_color_pair_) :
            COLOR_PAIR(exclude_filter_color_pair_));
    const auto& type =
        filter->is_include_filter() ? kIncludeType : kExcludeType;
    mvwaddstr(
        window_.get(),
        cur_row, 0,
        type.data());
    wclrtoeol(window_.get());
    if (max_pattern_len) {
      mvwaddnstr(
          window_.get(),
          cur_row, kTypeColumnSize,
          filter->pattern().c_str(),
          max_pattern_len);
    }
    mvwprintw(
        window_.get(),
        cur_row, filtered_out_start,
        "%zi",
        filter->filtered_out_records_count());
    ++cur_row;
  }
  wbkgdset(window_.get(), old_bkgd);
}

void FilterListWindow::DisplayBorders() noexcept {
  const int max_x = getmaxx(window_.get());
  mvwhline(window_.get(), 0, 0, 0, num_columns_);
  mvwhline(window_.get(), active_filters_.size() + 1, 0, 0, num_columns_);
  const std::string_view kPattern = " Filter pattern ";
  const std::string_view kRecordsExluded = " # records excl. ";
  const std::string_view kType = " Type ";

  mvwaddstr(window_.get(), 0, 1, kType.data());

  const int pattern_start = std::max<int>(0, max_x - kPattern.size()) / 2;
  mvwaddstr(window_.get(), 0, pattern_start, kPattern.data());

  const int records_excl_start = std::max<int>(
      0, max_x - kRecordsExluded.size());
  mvwaddstr(window_.get(), 0, records_excl_start, kRecordsExluded.data());
}

void FilterListWindow::FilterSetChanged(
    const std::vector<LogPatternFilter*>& active_filters) noexcept {
  active_filters_ = active_filters;
}

}  // namespace oko
