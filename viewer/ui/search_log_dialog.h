// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <form.h>

#include <memory>
#include <string>

#include "viewer/ui/dialog_window.h"
#include "viewer/ui/log_files_window.h"

namespace oko {

class SearchLogDialog : public DialogWindow {
 public:
  explicit SearchLogDialog(LogFilesWindow* window) noexcept;

 private:
  bool HandleEnter() noexcept override;
  std::string GetTitle() const noexcept override;

  LogFilesWindow* log_files_window_;
};

}  // namespace oko
