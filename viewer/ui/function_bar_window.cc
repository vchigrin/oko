// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/function_bar_window.h"

#include <utility>

#include "viewer/ui/color_manager.h"
#include "viewer/ui/ncurses_helpers.h"

namespace oko {

FunctionBarWindow::FunctionBarWindow(
    int start_row,
    int start_col,
    int num_columns) noexcept
    : Window(start_row, start_col, kRows, num_columns) {
  ColorManager& cm = ColorManager::instance();
  label_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_CYAN);
  key_name_color_pair_ = cm.RegisterColorPair(COLOR_WHITE, COLOR_BLACK);
}

void FunctionBarWindow::DisplayImpl() noexcept {
  wmove(window_.get(), 0, 0);
  for (int i = 0; i < kMaxFunctionKeys; ++i) {
    if (labels_[i].empty()) {
      continue;
    }
    {
      WithColor color(window_, key_name_color_pair_);
      wprintw(window_.get(), "F%d", i + 1);
    }
    {
      WithColor color(window_, label_color_pair_);
      waddstr(window_.get(), labels_[i].c_str());
    }
  }
  wclrtoeol(window_.get());
}

void FunctionBarWindow::HandleKeyPress(int key) noexcept {
}

}  // namespace oko
