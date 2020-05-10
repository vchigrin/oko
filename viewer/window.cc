// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/window.h"

#include <cassert>

namespace oko {

Window::Window(int start_row,
               int start_col,
               int num_rows,
               int num_columns) noexcept
      : window_(newwin(num_rows, num_columns, start_row, start_col),
                &delwin),
        start_row_(start_row),
        start_col_(start_col),
        num_rows_(num_rows),
        num_columns_(num_columns) {
  assert(window_);
}

void Window::Display() noexcept {
  DisplayImpl();
  wrefresh(window_.get());
}

void Window::Move(
    int start_row, int start_col, int num_rows, int num_columns) noexcept {
  assert(start_row >= 0);
  assert(start_col >= 0);
  assert(num_rows > 0);
  assert(num_columns > 0);
  start_row_ = start_row;
  start_col_ = start_col;
  num_rows_ = num_rows;
  num_columns_ = num_columns;
  window_.reset(
      newwin(num_rows_, num_columns_, start_row_, start_col_));
}

}  // namespace oko
