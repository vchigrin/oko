// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <memory>
#include <utility>
#include <vector>

#include "viewer/app_model.h"
#include "viewer/log_filter.h"
#include "viewer/ui/window.h"

namespace oko {

class FilterListWindow : public Window {
 public:
  FilterListWindow(
      AppModel* model,
      int start_row,
      int start_col,
      int num_columns) noexcept;

  int GetDesiredHeight() noexcept;

  void HandleKeyPress(int key) noexcept override;

 private:
  void FilterSetChanged(
      const std::vector<std::unique_ptr<LogFilter>>&
          new_active_filters) noexcept;
  void DisplayImpl() noexcept override;
  void DisplayBorders() noexcept;

  AppModel* app_model_;
  boost::signals2::scoped_connection filter_set_changed_conn_;
  std::vector<LogFilter*> active_filters_;
  int include_pattern_filter_color_pair_ = 0;
  int exclude_pattern_filter_color_pair_ = 0;
  int include_level_filter_color_pair_ = 0;
};

}  // namespace oko
