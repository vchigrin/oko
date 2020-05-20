// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <functional>
#include <string>

#include "viewer/ui/window.h"

namespace oko {

class MessageWindow : public Window {
 public:
  explicit MessageWindow(std::string message) noexcept;

  void PostSync() noexcept;

 private:
  void HandleKeyPress(int key) noexcept override;
  void DisplayImpl() noexcept override;

  const std::string message_;
  int button_color_pair_ = 0;
};

}  // namespace oko
