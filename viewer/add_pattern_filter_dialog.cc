// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/add_pattern_filter_dialog.h"

#include <algorithm>
#include <utility>
#include <boost/algorithm/string/trim.hpp>

namespace oko {

AddPatternFilterDialog::AddPatternFilterDialog(
    AppModel* model,
    bool is_include_filter) noexcept
    : DialogWindow(1),
      app_model_(model),
      is_include_filter_(is_include_filter) {
  fields_[0] = new_field(1, std::max(1, width_ - 4), 1, 1, 0, 0);
  set_field_back(fields_[0], A_REVERSE);
  // Make field dynamically resizing to allow strings longer then
  // field width.
  field_opts_off(fields_[0], O_STATIC);
  InitForm();
}

bool AddPatternFilterDialog::HandleEnter() noexcept {
  std::string entered_string = field_buffer(fields_[0], 0);
  boost::algorithm::trim_right(entered_string);
  if (!entered_string.empty()) {
    app_model_->AppendFilter(
        std::move(entered_string),
        is_include_filter_);
  }
  return true;
}

std::string AddPatternFilterDialog::GetTitle() const noexcept {
  return is_include_filter_ ?
      "Include fields with pattern" :
      "Exlude fields with pattern";
}

}  // namespace oko
