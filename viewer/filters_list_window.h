// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <utility>
#include <vector>

#include "viewer/window.h"
#include "viewer/log_pattern_filter.h"

namespace oko {

class FilterListWindow : public Window {
 public:
  FilterListWindow(
      int start_row,
      int start_col,
      int num_columns) noexcept;

  void UpdateActiveFilters(
      std::vector<LogPatternFilter*> active_filters) noexcept {
    active_filters_ = std::move(active_filters);
  }

  int GetDesiredHeight() noexcept;

  void HandleKeyPress(int key) noexcept override;

 private:
  void DisplayImpl() noexcept override;

  std::vector<LogPatternFilter*> active_filters_;
  int include_filter_color_pair_ = 0;
  int exclude_filter_color_pair_ = 0;
};

}  // namespace oko
