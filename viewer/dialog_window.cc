// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/dialog_window.h"

#include <algorithm>

namespace oko {

namespace {

const int kDesiredWindowWidth = 70;
const int kDesiredWindowHeight = 5;
const int kEscape = 27;

}  // namespace

DialogWindow::DialogWindow(size_t field_count) noexcept
    : Window(1, 1, 1, 1),
      subwindow_(nullptr, &delwin),
      fields_(field_count + 1, nullptr),
      form_(nullptr, &free_form) {
  int max_row = 0, max_col = 0;
  getmaxyx(stdscr, max_row, max_col);
  width_ = std::min(kDesiredWindowWidth, max_col);
  // Can not shrink vertically.
  height_ = kDesiredWindowHeight;
  const int start_col = std::max(0, (max_col - width_) / 2);
  const int start_row = std::max(0, (max_row - height_) / 2);
  Move(start_row, start_col, height_, width_);
  subwindow_.reset(derwin(window_.get(), height_ - 2, width_ - 2, 1, 1));
  // Very visible cursor
  prev_cursor_state_ = curs_set(2);
}

void DialogWindow::InitForm() noexcept {
  form_.reset(new_form(fields_.data()));
  set_form_win(form_.get(), window_.get());
  set_form_sub(form_.get(), subwindow_.get());
  post_form(form_.get());
  refresh();
}

DialogWindow::~DialogWindow() {
  unpost_form(form_.get());
  free_field(fields_[0]);
  curs_set(prev_cursor_state_);
}

void DialogWindow::DisplayImpl() noexcept {
  box(window_.get(), 0, 0);
  std::string title = GetTitle();
  title = " " + title + " ";
  int max_x = getmaxx(window_.get());
  int title_x = std::max(0, max_x - static_cast<int>(title.length())) / 2;
  int y = 0, x = 0;
  getyx(window_.get(), y, x);
  mvwaddstr(window_.get(), 0, title_x, title.c_str());
  wmove(window_.get(), y, x);
}

void DialogWindow::HandleKeyPress(int key) noexcept {
  switch (key) {
    case KEY_HOME:
      form_driver(form_.get(), REQ_BEG_LINE);
      break;
    case KEY_END:
      form_driver(form_.get(), REQ_END_LINE);
      break;
    case KEY_LEFT:
      form_driver(form_.get(), REQ_PREV_CHAR);
      break;
    case KEY_RIGHT:
      form_driver(form_.get(), REQ_NEXT_CHAR);
      break;
    case KEY_BACKSPACE:
      form_driver(form_.get(), REQ_DEL_PREV);
      break;
    case KEY_DC:
      form_driver(form_.get(), REQ_DEL_CHAR);
      break;
    case KEY_ENTER:
    case '\n':
      form_driver(form_.get(), REQ_VALIDATION);
      if (HandleEnter()) {
        finished_ = true;
      }
      break;
    case '\t':
      form_driver(form_.get(), REQ_NEXT_FIELD);
      break;
    case kEscape:
      finished_ = true;
      break;
    default:
      form_driver(form_.get(), key);
  }
}

}  // namespace oko
