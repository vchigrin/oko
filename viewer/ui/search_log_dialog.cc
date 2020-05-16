// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/search_log_dialog.h"

#include <algorithm>
#include <utility>
#include <boost/algorithm/string/trim.hpp>

namespace oko {

SearchLogDialog::SearchLogDialog(LogFilesWindow* window) noexcept
    : DialogWindow(1),
      log_files_window_(window) {
  fields_[0] = new_field(1, std::max(1, width_ - 4), 1, 1, 0, 0);
  set_field_buffer(fields_[0], 0, "*");
  set_field_back(fields_[0], A_REVERSE);
  // Make field dynamically resizing to allow strings longer then
  // field width.
  field_opts_off(fields_[0], O_STATIC);
  InitForm();
}

bool SearchLogDialog::HandleEnter() noexcept {
  std::string entered_string = field_buffer(fields_[0], 0);
  boost::algorithm::trim_right(entered_string);
  if (!entered_string.empty()) {
    log_files_window_->SearchForFilesByMask(std::move(entered_string));
  }
  return true;
}

std::string SearchLogDialog::GetTitle() const noexcept {
  return "File mask to search for";
}

}  // namespace oko
