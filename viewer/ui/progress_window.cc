// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/progress_window.h"

#include <algorithm>
#include <boost/format.hpp>
#include <chrono>
#include <thread>
#include <vector>
#include <utility>

#include "viewer/ui/color_manager.h"
#include "viewer/ui/ncurses_helpers.h"

namespace oko {

namespace {

const int kDesiredWindowWidth = 70;
const int kSliderWidth = 2;
const int kDesiredWindowHeight = 7;
const int kProgressRow = 2;
const int kTextRow = 4;

}  // namespace

ProgressWindow::ProgressWindow(
    const std::string& title, IsReadyCallback cb) noexcept
    : Window(1, 1, 1, 1),
      title_(" " + title + " "),
      is_ready_callback_(std::move(cb)) {
  int max_row = 0, max_col = 0;
  getmaxyx(stdscr, max_row, max_col);
  int width = std::min(kDesiredWindowWidth, max_col);
  progress_width_ = std::max(0, width - 4);
  // Can not shrink vertically.
  const int start_col = std::max(0, (max_col - width) / 2);
  const int start_row = std::max(0, (max_row - kDesiredWindowHeight) / 2);
  Move(start_row, start_col, kDesiredWindowHeight, width);

  ColorManager& cm = ColorManager::instance();
  bg_progress_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_YELLOW);
  fg_progress_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_RED);
}

void ProgressWindow::HandleKeyPress(int key) noexcept {
}

void ProgressWindow::DisplayImpl() noexcept {
  DrawBorder();
  DrawProgress();
  DrawTimePassed();
}

void ProgressWindow::DrawProgress() noexcept {
  if (progress_width_ <= 0) {
    return;
  }
  std::vector<char> buf(progress_width_, ' ');
  int progress_start_x = 2;
  {
    WithColor bg(window_, bg_progress_color_pair_);
    mvwaddnstr(window_.get(), kProgressRow,
        progress_start_x, buf.data(), buf.size());
  }
  {
    WithColor fg(window_, fg_progress_color_pair_);
    buf.resize(kSliderWidth);
    mvwaddnstr(
        window_.get(), kProgressRow,
        progress_start_x + phase_, buf.data(), buf.size());
  }
}

void ProgressWindow::DrawTimePassed() noexcept {
  const std::string text = boost::str(
      boost::format("%1% sec. passed") % passed_sec_);
  int start_x = std::max(0, num_columns_ - static_cast<int>(text.size())) / 2;
  mvwaddstr(window_.get(), kTextRow, start_x, text.c_str());
}

void ProgressWindow::DrawBorder() noexcept {
  box(window_.get(), 0, 0);
  int title_x = std::max(
      0, num_columns_ - static_cast<int>(title_.length())) / 2;
  mvwaddstr(window_.get(), 0, title_x, title_.c_str());
}

void ProgressWindow::PostSync() noexcept {
  Display();
  const auto start_time = std::chrono::steady_clock::now();
  while (!is_ready_callback_()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    phase_ = phase_ + direction_;
    if (phase_ + kSliderWidth >= progress_width_) {
      direction_ = -1;
      phase_ = std::max(0, progress_width_ - kSliderWidth);
    } else if (phase_ < 0) {
      direction_ = 1;
      phase_ = 0;
    }
    passed_sec_ = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start_time).count();
    DrawProgress();
    DrawTimePassed();
    wrefresh(window_.get());
  }
}

}  // namespace oko
