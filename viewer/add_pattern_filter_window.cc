// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/add_pattern_filter_window.h"

#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include <iostream>

namespace oko {

namespace {

const int kDesiredWindowWidth = 70;
const int kDesiredWindowHeight = 5;

}  // namespace

AddPatternFilterWindow::AddPatternFilterWindow(bool is_include_filter) noexcept
    : Window(1, 1, 1, 1),
      subwindow_(nullptr, &delwin),
      is_include_filter_(is_include_filter),
      form_(nullptr, &free_form) {
  int max_row = 0, max_col = 0;
  getmaxyx(stdscr, max_row, max_col);
  const int width = std::min(kDesiredWindowWidth, max_col);
  // Can not shrink vertically.
  const int height = kDesiredWindowHeight;
  const int start_col = (max_col - width) / 2;
  const int start_row = (max_row - height) / 2;
  Move(start_row, start_col, height, width);
  subwindow_.reset(derwin(window_.get(), height - 2, width - 2, 1, 1));

  fields_[0] = new_field(1, std::max(1, width - 4), 1, 1, 0, 0);
  set_field_back(fields_[0], A_REVERSE);
  // Make field dynamically resizing to allow strings longer then
  // field width.
  field_opts_off(fields_[0], O_STATIC);
  fields_[1] = nullptr;
  form_.reset(new_form(fields_));
  set_form_win(form_.get(), window_.get());
  set_form_sub(form_.get(), subwindow_.get());
  post_form(form_.get());
  // Very visible cursor
  prev_cursor_state_ = curs_set(2);
  refresh();
}

AddPatternFilterWindow::~AddPatternFilterWindow() {
  unpost_form(form_.get());
  free_field(fields_[0]);
  curs_set(prev_cursor_state_);
}

void AddPatternFilterWindow::DisplayImpl() noexcept {
  box(window_.get(), 0, 0);
  const std::string label = is_include_filter_ ?
      " Include fields with pattern " :
      " Exlude fields with pattern ";
  int max_x = getmaxx(window_.get());
  int label_x = std::max(0, max_x - static_cast<int>(label.length())) / 2;
  int y = 0, x = 0;
  getyx(window_.get(), y, x);
  mvwaddstr(window_.get(), 0, label_x, label.c_str());
  wmove(window_.get(), y, x);
}

void AddPatternFilterWindow::HandleKeyPress(int key) noexcept {
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
      finished_ = true;
      form_driver(form_.get(), REQ_VALIDATION);
      entered_string_ = field_buffer(fields_[0], 0);
      boost::algorithm::trim_right(entered_string_);
      break;
    default:
      form_driver(form_.get(), key);
  }
}

}  // namespace oko
