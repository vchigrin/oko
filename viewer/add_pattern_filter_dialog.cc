// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/add_pattern_filter_dialog.h"

#include <algorithm>
#include <boost/algorithm/string/trim.hpp>

namespace oko {

AddPatternFilterDialog::AddPatternFilterDialog(bool is_include_filter) noexcept
    : DialogWindow(1),
      is_include_filter_(is_include_filter) {
  fields_[0] = new_field(1, std::max(1, width_ - 4), 1, 1, 0, 0);
  set_field_back(fields_[0], A_REVERSE);
  // Make field dynamically resizing to allow strings longer then
  // field width.
  field_opts_off(fields_[0], O_STATIC);
  InitForm();
}

bool AddPatternFilterDialog::HandleEnter() noexcept {
  entered_string_ = field_buffer(fields_[0], 0);
  boost::algorithm::trim_right(entered_string_);
  return true;
}

std::string AddPatternFilterDialog::GetTitle() const noexcept {
  return is_include_filter_ ?
      "Include fields with pattern" :
      "Exlude fields with pattern";
}

}  // namespace oko
