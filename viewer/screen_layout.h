// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once

#include "viewer/filters_list_window.h"
#include "viewer/log_window.h"
#include "viewer/status_window.h"

namespace oko {

class ScreenLayout {
 public:
  explicit ScreenLayout(LogView* view) noexcept;
  void Display() noexcept;
  void RecalcPositions() noexcept;
  void HandleKeyPress(int key) noexcept;

  oko::FilterListWindow& filter_list_window() noexcept {
    return filter_list_window_;
  }

  oko::LogWindow& log_window() noexcept {
    return log_window_;
  }

  oko::StatusWindow& status_window() noexcept {
    return status_window_;
  }

 private:
  LogWindow log_window_;
  FilterListWindow filter_list_window_;
  StatusWindow status_window_;
};

}  // namespace oko
