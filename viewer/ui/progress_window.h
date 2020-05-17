// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <functional>
#include <string>

#include "viewer/ui/window.h"

namespace oko {

class ProgressWindow : public Window {
 public:
  using IsReadyCallback = std::function<bool()>;

  ProgressWindow(const std::string& title, IsReadyCallback cb) noexcept;

  void PostSync() noexcept;

 private:
  void HandleKeyPress(int key) noexcept override;
  void DisplayImpl() noexcept override;
  void DrawBorder() noexcept;
  void DrawProgress() noexcept;
  void DrawTimePassed() noexcept;

  const std::string title_;
  const IsReadyCallback is_ready_callback_;
  int phase_ = 0;
  int direction_ = 1;
  int passed_sec_ = 0;
  int progress_width_ = 0;
  int bg_progress_color_pair_ = 0;
  int fg_progress_color_pair_ = 0;
};

}  // namespace oko
