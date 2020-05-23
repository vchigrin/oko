// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/add_level_filter_dialog.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "viewer/log_level_filter.h"
#include "viewer/ui/ncurses_helpers.h"

namespace oko {

namespace {

const int kDesiredWindowWidth = 30;

}  // namespace

AddLevelFilterDialog::AddLevelFilterDialog(AppModel* model)
    : DialogWindow(0),
      app_model_(model) {
  // Initially all levels are checked.
  for (int level = static_cast<int>(LogLevel::Invalid) + 1;
      level < static_cast<int>(LogLevel::AfterLast); ++level) {
    checked_levels_.insert(static_cast<LogLevel>(level));
  }

  int max_row = 0, max_col = 0;
  getmaxyx(stdscr, max_row, max_col);
  const int width = std::min(kDesiredWindowWidth, max_col);
  const int level_count = static_cast<int>(LogLevel::AfterLast) -
      static_cast<int>(LogLevel::Invalid) - 1;
  const int height = level_count + 2;

  // Can not shrink vertically.
  const int start_col = std::max(0, (max_col - width) / 2);
  const int start_row = std::max(0, (max_row - height) / 2);
  Move(start_row, start_col, height, width);
}

void AddLevelFilterDialog::HandleKeyPress(int key) noexcept {
  switch (key) {
    case KEY_ENTER:
    case '\n':
      HandleEnter();
      finished_ = true;
      break;
    case kEscape:
      finished_ = true;
      break;
    case 'j':
    case KEY_DOWN:
    {
      int next_level = static_cast<int>(selected_level_) + 1;
      if (next_level >= static_cast<int>(LogLevel::AfterLast)) {
        next_level = static_cast<int>(LogLevel::Invalid) + 1;
      }
      selected_level_ = static_cast<LogLevel>(next_level);
      break;
    }
    case 'k':
    case KEY_UP:
    {
      int next_level = static_cast<int>(selected_level_) - 1;
      if (next_level <= static_cast<int>(LogLevel::Invalid)) {
        next_level = static_cast<int>(LogLevel::AfterLast) - 1;
      }
      selected_level_ = static_cast<LogLevel>(next_level);
      break;
    }
    case ' ':
    {
      if (checked_levels_.count(selected_level_) > 0) {
        checked_levels_.erase(selected_level_);
      } else {
        checked_levels_.insert(selected_level_);
      }
    }
  }
}

void AddLevelFilterDialog::DisplayImpl() noexcept {
  DialogWindow::DisplayImpl();
  int cur_row = 0;
  for (int l = static_cast<int>(LogLevel::Invalid) + 1;
      l < static_cast<int>(LogLevel::AfterLast); ++l, ++cur_row) {
    std::stringstream label;
    const LogLevel level = static_cast<LogLevel>(l);
    const bool checked = checked_levels_.count(level) > 0;
    label << "[" << (checked ? "x" : " ") << "] " << level;
    if (level == selected_level_) {
      wattron(subwindow_.get(), A_REVERSE);
    }
    mvwaddstr(subwindow_.get(), cur_row, 1, label.str().c_str());
    if (level == selected_level_) {
      wattroff(subwindow_.get(), A_REVERSE);
    }
  }
}

bool AddLevelFilterDialog::HandleEnter() noexcept {
  std::unique_ptr<LogLevelFilter> new_filter =
      std::make_unique<LogLevelFilter>(
          &app_model_->active_view(),
          std::move(checked_levels_));
  app_model_->AppendFilter(std::move(new_filter));
  return true;
}

std::string AddLevelFilterDialog::GetTitle() const noexcept {
  return "Include log levels";
}


}  // namespace oko
