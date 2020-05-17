// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <cassert>
#include <string>
#include <utility>

#include "viewer/ui/window.h"


namespace oko {

// Always fixed height function bar window, usually at the bottom of screen.
class FunctionBarWindow : public Window {
 public:
  static constexpr int kRows = 1;

  FunctionBarWindow(
      int start_row,
      int start_col,
      int num_columns) noexcept;

  // Here number is F-key number (one-based).
  void SetLabel(int number, std::string label) noexcept {
    assert(number > 0 && number <= kMaxFunctionKeys);
    labels_[number - 1] = std::move(label);
  }

 private:
  static const int kMaxFunctionKeys = 12;

  // Does nothing.
  void HandleKeyPress(int key) noexcept override;

  void DisplayImpl() noexcept override;

  int label_color_pair_ = 0;
  int key_name_color_pair_ = 0;

  // Zero element corresponds to F1, first to F2, etc.
  std::string labels_[kMaxFunctionKeys];
};

}  // namespace oko
