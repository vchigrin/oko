// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/message_window.h"

#include <algorithm>
#include <utility>

#include "viewer/ui/color_manager.h"
#include "viewer/ui/ncurses_helpers.h"

namespace oko {

namespace {

constexpr int kMargin = 2;
constexpr int kButtonRows = 2;

std::pair<size_t, size_t> MeasureMessage(const std::string& message) noexcept {
  size_t width = 0;
  size_t height = 0;
  size_t line_start_index = 0;
  while (line_start_index < message.size()) {
    size_t line_end_index = message.find('\n', line_start_index);
    if (line_end_index == std::string::npos) {
      line_end_index = message.size();
    }
    size_t cur_line_size = line_end_index - line_start_index;
    ++height;
    width = std::max(width, cur_line_size);
    line_start_index = line_end_index + 1;
  }
  return std::pair(width, height);
}

}  // namespace

MessageWindow::MessageWindow(std::string message) noexcept
    : Window(1, 1, 1, 1),
      message_(std::move(message)),
      text_window_(nullptr, &delwin) {
  int max_row = 0, max_col = 0;
  getmaxyx(stdscr, max_row, max_col);
  auto [message_width, message_height] = MeasureMessage(message_);
  int width = std::min<int>(message_width + 2 * kMargin, max_col);
  int height = std::min<int>(
      message_height + 2 * kMargin + kButtonRows, max_row);
  int start_col = (max_col - width) / 2;
  int start_row = (max_row - height) / 2;
  Move(start_row, start_col, height, width);
  ColorManager& cm = ColorManager::instance();
  button_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_WHITE);
  text_window_.reset(
      derwin(window_.get(), message_height + 1, message_width + 1, 2, 2));
}

// static
void MessageWindow::PostSync(std::string message) noexcept {
  MessageWindow wnd(std::move(message));
  wnd.Display();
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
  mvwaddstr(text_window_.get(), 0, 0, message_.c_str());
  wrefresh(text_window_.get());
  static const std::string_view kOk = " OK ";
  int button_col = std::max<int>(0, num_columns_ - kOk.size()) / 2;
  WithColor colorer(window_, button_color_pair_);
  mvwaddnstr(
      window_.get(),
      num_rows_ - kButtonRows - 1, button_col, kOk.data(), kOk.size());
}

}  // namespace oko
