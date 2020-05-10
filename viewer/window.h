// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <ncurses.h>

#include <memory>

namespace oko {

// Abstract base class for all windows.
class Window {
 public:
  Window(int start_row,
         int start_col,
         int num_rows,
         int num_columns) noexcept;

  virtual ~Window() = default;

  virtual void Move(
      int start_row, int start_col,
      int num_rows, int num_columns) noexcept;

  // Calls DisplayImpl and then wrefresh();
  virtual void Display() noexcept final;
  virtual void HandleKeyPress(int key) noexcept = 0;

 protected:
  virtual void DisplayImpl() noexcept = 0;

  std::unique_ptr<WINDOW, int(*)(WINDOW*)> window_;
  int start_row_;
  int start_col_;
  int num_rows_;
  int num_columns_;
};

}  // namespace oko

