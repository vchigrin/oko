// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <string>

#include "viewer/app_model.h"
#include "viewer/log_record.h"
#include "viewer/ui/window.h"


namespace oko {

// Always fixed height status window, usually at the bottom of screen.
class StatusWindow : public Window {
 public:
  static constexpr int kRows = 2;

  StatusWindow(
      AppModel* model,
      int start_row,
      int start_col,
      int num_columns) noexcept;

  void HandleKeyPress(int key) noexcept override;

 private:
  AppModel* app_model_;
  void DisplayImpl() noexcept override;

  int status_color_pair_ = 0;
  int status_mark_color_pair_ = 0;
};

}  // namespace oko
