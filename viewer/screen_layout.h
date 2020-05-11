// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once

#include <vector>

#include "viewer/app_model.h"
#include "viewer/filters_list_window.h"
#include "viewer/log_window.h"
#include "viewer/status_window.h"

namespace oko {

class ScreenLayout {
 public:
  explicit ScreenLayout(AppModel* model) noexcept;
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
  void FilterSetChanged(
      const std::vector<LogPatternFilter*>& active_filters);

  AppModel* app_model_;
  LogWindow log_window_;
  FilterListWindow filter_list_window_;
  StatusWindow status_window_;
  boost::signals2::scoped_connection filter_set_changed_conn_;
};

}  // namespace oko
