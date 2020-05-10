// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <form.h>

#include <memory>
#include <string>

#include "viewer/window.h"

namespace oko {

class AddPatternFilterWindow : public Window {
 public:
  explicit AddPatternFilterWindow(bool is_include_filter) noexcept;
  ~AddPatternFilterWindow();

  void HandleKeyPress(int key) noexcept override;

  bool finished() const noexcept {
    return finished_;
  }

  bool is_include_filter() const noexcept {
    return is_include_filter_;
  }

  const std::string& entered_string() const noexcept {
    return entered_string_;
  }

 private:
  void DisplayImpl() noexcept override;

  std::unique_ptr<WINDOW, int(*)(WINDOW*)> subwindow_;

  const bool is_include_filter_;
  bool finished_ = false;
  std::string entered_string_;
  FIELD* fields_[2];
  std::unique_ptr<FORM, int(*)(FORM*)> form_;
  int prev_cursor_state_ = 0;
};

}  // namespace oko
