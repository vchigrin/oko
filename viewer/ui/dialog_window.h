// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <form.h>

#include <memory>
#include <string>
#include <vector>

#include "viewer/ui/window.h"

namespace oko {

class DialogWindow : public Window {
 public:
  // |field_count| counts only real fields, without trailint nullptr
  // |fields_| entry.
  explicit DialogWindow(size_t field_count) noexcept;
  virtual ~DialogWindow();

  void HandleKeyPress(int key) noexcept override;

  bool finished() const noexcept {
    return finished_;
  }

 protected:
  // Called when user pressed |Enter| in dialog. Returning true causes
  // dialog to become "finished()".
  virtual bool HandleEnter() noexcept = 0;
  virtual std::string GetTitle() const noexcept = 0;
  // Must be called by child classes after it finished filled fields_ vector.
  void InitForm() noexcept;
  void DisplayImpl() noexcept override;

  std::unique_ptr<WINDOW, int(*)(WINDOW*)> subwindow_;

  bool finished_ = false;
  int width_ = 0;
  int height_ = 0;
  std::vector<FIELD*> fields_;
  std::unique_ptr<FORM, int(*)(FORM*)> form_;
  int prev_cursor_state_ = 0;
};

}  // namespace oko
