// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <string>
#include <unordered_set>

#include "viewer/app_model.h"
#include "viewer/log_record.h"
#include "viewer/ui/dialog_window.h"

namespace oko {

class AddLevelFilterDialog : public DialogWindow {
 public:
  explicit AddLevelFilterDialog(AppModel* model);

 private:
  void HandleKeyPress(int key) noexcept override;
  void DisplayImpl() noexcept override;
  bool HandleEnter() noexcept override;
  std::string GetTitle() const noexcept override;

  AppModel* const app_model_;
  LogLevel selected_level_ = LogLevel::Debug;
  std::unordered_set<LogLevel> checked_levels_;
};

}  // namespace oko
