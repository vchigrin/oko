// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/search_dialog.h"

#include <algorithm>
#include <utility>
#include <boost/algorithm/string/trim.hpp>

namespace oko {

SearchDialog::SearchDialog(AppModel* model) noexcept
    : DialogWindow(1),
      app_model_(model) {
  fields_[0] = new_field(1, std::max(1, width_ - 4), 1, 1, 0, 0);
  set_field_back(fields_[0], A_REVERSE);
  // Make field dynamically resizing to allow strings longer then
  // field width.
  field_opts_off(fields_[0], O_STATIC);
  InitForm();
}

bool SearchDialog::HandleEnter() noexcept {
  std::string entered_string = field_buffer(fields_[0], 0);
  boost::algorithm::trim_right(entered_string);
  if (!entered_string.empty()) {
    app_model_->SearchForMessage(std::move(entered_string));
  }
  return true;
}

std::string SearchDialog::GetTitle() const noexcept {
  return "Search for message";
}

}  // namespace oko
