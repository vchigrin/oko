// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/filters_list_window.h"

#include <algorithm>
#include <string>
#include <unordered_set>

#include "viewer/log_level_filter.h"
#include "viewer/log_pattern_filter.h"
#include "viewer/ui/color_manager.h"
#include "viewer/ui/ncurses_helpers.h"

namespace oko {

namespace {
const int kFilteredOutColumnSize = 16;
const std::string_view kIncludePatternStr = "Incl pattern:";
const std::string_view kExcludePatternStr = "Excl pattern:";
const std::string_view kIncludeLevelStr = "Incl levels:";

const int kTypeColumnSize = std::max({
    kIncludePatternStr.size(),
    kExcludePatternStr.size(),
    kIncludeLevelStr.size()
});

std::string LevelSetToString(
    const std::unordered_set<LogLevel>& levels) {
  std::vector<LogLevel> sorted_levels{levels.begin(), levels.end()};
  std::sort(sorted_levels.begin(), sorted_levels.end());

  std::stringstream result;
  bool first = true;
  for (LogLevel level : sorted_levels) {
    if (!first) {
      result << " ";
    }
    result << level;
    first = false;
  }
  return result.str();
}

}  // namespace

FilterListWindow::FilterListWindow(
    AppModel* model,
    int start_row,
    int start_col,
    int num_columns) noexcept
    : Window(start_row, start_col, 1, num_columns),
      app_model_(model) {
  ColorManager& cm = ColorManager::instance();
  include_pattern_filter_color_pair_ = cm.RegisterColorPair(
      COLOR_BLACK, COLOR_GREEN);
  exclude_pattern_filter_color_pair_ = cm.RegisterColorPair(
      COLOR_BLACK, COLOR_RED);
  include_level_filter_color_pair_ = cm.RegisterColorPair(
      COLOR_BLACK, COLOR_BLUE);

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
  const int max_description_len = std::max(
      0, max_x - kFilteredOutColumnSize - kTypeColumnSize);
  const int filtered_out_start = max_x - kFilteredOutColumnSize;

  for (LogFilter* filter : active_filters_) {
    const std::string_view* type_str = nullptr;
    int color = -1;
    std::string description;
    switch (filter->type()) {
      case LogFilter::Type::kIncludePattern:
        type_str = &kIncludePatternStr;
        color = include_pattern_filter_color_pair_;
        description = static_cast<LogPatternFilter*>(filter)->pattern();
        break;
      case LogFilter::Type::kExcludePattern:
        type_str = &kExcludePatternStr;
        color = exclude_pattern_filter_color_pair_;
        description = static_cast<LogPatternFilter*>(filter)->pattern();
        break;
      case LogFilter::Type::kLevel:
        type_str = &kIncludeLevelStr;
        color = include_level_filter_color_pair_;
        description = LevelSetToString(
            static_cast<LogLevelFilter*>(filter)->levels_to_include());
        break;
    }

    wbkgdset(window_.get(), COLOR_PAIR(color));
    mvwaddstr(window_.get(), cur_row, 0, type_str->data());
    wclrtoeol(window_.get());
    if (max_description_len) {
      mvwaddnstr(
          window_.get(),
          cur_row, kTypeColumnSize,
          description.c_str(),
          max_description_len);
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
    const std::vector<std::unique_ptr<LogFilter>>&
        new_active_filters) noexcept {
  active_filters_.resize(new_active_filters.size());
  std::transform(
      new_active_filters.begin(),
      new_active_filters.end(),
      active_filters_.begin(),
      [](const std::unique_ptr<LogFilter>& f) {
        return f.get();
      });
}

}  // namespace oko
