// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <form.h>

#include <optional>
#include <string>

#include "viewer/app_model.h"
#include "viewer/ui/dialog_window.h"

namespace oko {

class GoToTimestampDialog : public DialogWindow {
 public:
  explicit GoToTimestampDialog(AppModel* model) noexcept;

 private:
  void DisplayImpl() noexcept override;
  bool HandleEnter() noexcept override;
  std::string GetTitle() const noexcept override;

  AppModel* app_model_;
  int margin_ = 0;
};

}  // namespace oko
