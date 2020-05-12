// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <form.h>

#include <memory>
#include <string>

#include "viewer/dialog_window.h"

namespace oko {

class AddPatternFilterDialog : public DialogWindow {
 public:
  explicit AddPatternFilterDialog(bool is_include_filter) noexcept;

  bool is_include_filter() const noexcept {
    return is_include_filter_;
  }

  const std::string& entered_string() const noexcept {
    return entered_string_;
  }

 private:
  bool HandleEnter() noexcept override;
  std::string GetTitle() const noexcept override;

  const bool is_include_filter_;
  std::string entered_string_;
};

}  // namespace oko
