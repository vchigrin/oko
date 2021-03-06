// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <form.h>

#include <memory>
#include <string>

#include "viewer/app_model.h"
#include "viewer/ui/dialog_window.h"

namespace oko {

class SearchDialog : public DialogWindow {
 public:
  explicit SearchDialog(AppModel* model) noexcept;

 private:
  bool HandleEnter() noexcept override;
  std::string GetTitle() const noexcept override;

  AppModel* app_model_;
};

}  // namespace oko
