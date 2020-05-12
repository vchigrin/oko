// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/go_to_timestamp_dialog.h"

#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include <charconv>
#include <limits>

#include "viewer/app_model.h"

namespace oko {

namespace {
const int kFieldWidth = 15;
const std::string_view kSecondsLabel = "S:";
const std::string_view kNanoSecondsLabel = " NS:";
}  // namespace

GoToTimestampDialog::GoToTimestampDialog(LogWindow* log_window) noexcept
    : DialogWindow(2),
      log_window_(log_window) {
  margin_ = std::max<int>(
      0,
      width_ - 2 * kFieldWidth - kSecondsLabel.size() -
          kNanoSecondsLabel.size()) / 2;
  int cur_x = margin_ + kSecondsLabel.size();
  // Seconds field
  fields_[0] = new_field(1, kFieldWidth, 1, cur_x, 0, 0);
  cur_x += kFieldWidth + kNanoSecondsLabel.size();
  set_field_back(fields_[0], A_REVERSE);
  set_field_type(fields_[0], TYPE_INTEGER, 0, 0, std::numeric_limits<int>::max);
  field_opts_off(fields_[0], O_AUTOSKIP);
  // Nanoseconds field
  fields_[1] = new_field(1, kFieldWidth, 1, cur_x, 0, 0);
  set_field_back(fields_[1], A_REVERSE);
  set_field_type(fields_[1], TYPE_INTEGER, 9, 0, 999999999);
  field_opts_off(fields_[1], O_AUTOSKIP);

  LogRecord::time_point selected_time_point =
      log_window_->GetSelectedRecordTimestamp();
  int sec_nsec[2];
  sec_nsec[0] = std::chrono::duration_cast<std::chrono::seconds>(
      selected_time_point.time_since_epoch()).count();
  sec_nsec[1] = std::chrono::duration_cast<std::chrono::nanoseconds>(
      selected_time_point.time_since_epoch() -
          std::chrono::seconds(sec_nsec[0])).count();
  for (int i = 0; i < 2; ++i) {
    char* buf = field_buffer(fields_[i], 0);
    std::to_chars(buf, buf + kFieldWidth, sec_nsec[i]);
  }

  InitForm();
}

void GoToTimestampDialog::DisplayImpl() noexcept {
  DialogWindow::DisplayImpl();
  int y = 0, x = 0;
  getyx(subwindow_.get(), y, x);
  mvwaddstr(subwindow_.get(), 1, margin_, kSecondsLabel.data());
  mvwaddstr(subwindow_.get(), 1, margin_ + kFieldWidth + kSecondsLabel.size(),
      kNanoSecondsLabel.data());
  wmove(subwindow_.get(), y, x);
}

bool GoToTimestampDialog::HandleEnter() noexcept {
  int sec_nsec[2];
  for (int i = 0; i < 2; ++i) {
    std::string buffer_data = field_buffer(fields_[i], 0);
    boost::algorithm::trim(buffer_data);
    auto res = std::from_chars(
        buffer_data.data(),
        buffer_data.data() + buffer_data.size(),
        sec_nsec[i]);
    if (res.ec != std::errc() ||
        res.ptr != buffer_data.data() + buffer_data.size()) {
      return false;
    }
  }
  LogRecord::time_point tp = LogRecord::time_point() +
      std::chrono::seconds(sec_nsec[0]) +
          std::chrono::nanoseconds(sec_nsec[1]);
  log_window_->SelectRecordByTimestamp(tp);
  return true;
}

std::string GoToTimestampDialog::GetTitle() const noexcept {
  return "Target timestamp";
}

}  // namespace oko
