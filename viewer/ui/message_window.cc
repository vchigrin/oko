// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/message_window.h"

#include <algorithm>

#include "viewer/ui/color_manager.h"
#include "viewer/ui/ncurses_helpers.h"

namespace oko {

namespace {

const int kDesiredWindowHeight = 7;
const int kMessageRow = 2;
const int kButtonRow = 5;

}  // namespace

MessageWindow::MessageWindow(std::string message) noexcept
    : Window(1, 1, 1, 1),
      message_(std::move(message)) {
  int max_row = 0, max_col = 0;
  getmaxyx(stdscr, max_row, max_col);
  int width = std::min<int>(message_.size() + 4, max_col);
  int start_col = (max_col - width) / 2;
  int start_row = (max_row - kDesiredWindowHeight) / 2;
  Move(start_row, start_col, kDesiredWindowHeight, width);
  ColorManager& cm = ColorManager::instance();
  button_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_WHITE);
}

void MessageWindow::PostSync() noexcept {
  Display();
  while (true) {
    int key = getch();
    if (key == KEY_ENTER || key == '\n' || key == kEscape) {
      break;
    }
  }
}

void MessageWindow::HandleKeyPress(int key) noexcept {
}

void MessageWindow::DisplayImpl() noexcept {
  box(window_.get(), 0, 0);
  int message_col = std::max<int>(0, num_columns_ - message_.size()) / 2;
  mvwaddstr(window_.get(), kMessageRow, message_col, message_.c_str());
  static const std::string_view kOk = " OK ";
  int button_col = std::max<int>(0, num_columns_ - kOk.size()) / 2;
  WithColor colorer(window_, button_color_pair_);
  mvwaddnstr(window_.get(), kButtonRow, button_col, kOk.data(), kOk.size());
}

}  // namespace oko
