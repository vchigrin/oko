// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <vector>

#include "viewer/app_model.h"
#include "viewer/ui/filters_list_window.h"
#include "viewer/ui/function_bar_window.h"
#include "viewer/ui/log_window.h"
#include "viewer/ui/status_window.h"

namespace oko {

class ScreenLayout {
 public:
  explicit ScreenLayout(AppModel* model) noexcept;
  void Display() noexcept;
  void RecalcPositions() noexcept;
  void HandleKeyPress(int key) noexcept;

  FilterListWindow& filter_list_window() noexcept {
    return filter_list_window_;
  }

  LogWindow& log_window() noexcept {
    return log_window_;
  }

  StatusWindow& status_window() noexcept {
    return status_window_;
  }

  FunctionBarWindow& function_bar_window() noexcept {
    return function_bar_window_;
  }

 private:
  void FilterSetChanged(
      const std::vector<std::unique_ptr<LogFilter>>& active_filters);

  AppModel* app_model_;
  LogWindow log_window_;
  FilterListWindow filter_list_window_;
  StatusWindow status_window_;
  FunctionBarWindow function_bar_window_;
  boost::signals2::scoped_connection filter_set_changed_conn_;
};

}  // namespace oko
