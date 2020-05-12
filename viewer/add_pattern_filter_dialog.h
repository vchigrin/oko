// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <form.h>

#include <memory>
#include <string>

#include "viewer/app_model.h"
#include "viewer/dialog_window.h"

namespace oko {

class AddPatternFilterDialog : public DialogWindow {
 public:
  AddPatternFilterDialog(
      AppModel* model,
      bool is_include_filter) noexcept;

 private:
  bool HandleEnter() noexcept override;
  std::string GetTitle() const noexcept override;

  AppModel* app_model_;
  const bool is_include_filter_;
};

}  // namespace oko
