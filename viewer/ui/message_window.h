// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <functional>
#include <memory>
#include <string>

#include "viewer/ui/window.h"

namespace oko {

class MessageWindow : public Window {
 public:
  static void PostSync(std::string message) noexcept;

 private:
  explicit MessageWindow(std::string message) noexcept;

  void HandleKeyPress(int key) noexcept override;
  void DisplayImpl() noexcept override;

  const std::string message_;
  std::unique_ptr<WINDOW, int(*)(WINDOW*)> text_window_;
  int button_color_pair_ = 0;
};

}  // namespace oko
