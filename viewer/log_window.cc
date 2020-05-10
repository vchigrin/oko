// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/log_window.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <optional>
#include <utility>
#include <vector>

#include "viewer/ncurses_helpers.h"

namespace oko {

namespace {
const int kTimeStartCol = 0;
// 10 digits for seconds + dot + 9 digits for nanoseconds.
const int kTimeColSize = 20;
const int kLevelStartCol = kTimeStartCol + kTimeColSize + 1;
const int kLevelColSize = 5;
const int kMessageStartCol = kLevelStartCol + kLevelColSize + 1;

}  // namespace

LogWindow::LogWindow(
    LogView* view,
    int start_row,
    int start_col,
    int num_rows,
    int num_columns) noexcept
    : Window(start_row, start_col, num_rows, num_columns),
      view_(view) {
}

void LogWindow::DisplayImpl() noexcept {
  const size_t after_last_record = GetDisplayedRecordAfterLast();
  const std::vector<LogRecord>& records = view_->GetRecords();
  for (size_t i = first_shown_record_, row = 0;
      i < after_last_record; ++i, ++row) {
    if (row == cursor_line_) {
      wattron(window_.get(), A_REVERSE);
    }
    const bool is_marked = IsMarked(i);
    if (is_marked) {
      wattron(window_.get(), COLOR_PAIR(kMarkColorPair));
    }
    DisplayTime(is_marked, row, records[i].timestamp());
    mvwaddch(window_.get(), row, kTimeStartCol + kTimeColSize, ' ');
    DisplayLevel(is_marked, row, records[i].log_level());
    mvwaddch(window_.get(), row, kLevelStartCol + kLevelColSize, ' ');
    DisplayMessage(row, records[i].message());
    wclrtoeol(window_.get());
    if (is_marked) {
      wattroff(window_.get(), COLOR_PAIR(kMarkColorPair));
    }
    if (row == cursor_line_) {
      wattroff(window_.get(), A_REVERSE);
    }
  }
}

void LogWindow::DisplayTime(
    bool is_marked,
    int row, const LogRecord::time_point time_point) noexcept {
  std::array<char, kTimeColSize + 1> buf;
  const int secs = std::chrono::duration_cast<std::chrono::seconds>(
      time_point.time_since_epoch()).count();
  const int nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(
      time_point.time_since_epoch() - std::chrono::seconds(secs)).count();
  char* next = buf.data();
  char* end_ptr = buf.data() + buf.size();
  auto to_char_res = std::to_chars(next, end_ptr, secs);
  if (to_char_res.ec != std::errc() ||
      to_char_res.ptr == end_ptr) {
    assert(false);
    return;
  }
  next = to_char_res.ptr;
  *next++ = '.';
  if (next == end_ptr) {
    assert(false);
    return;
  }
  to_char_res = std::to_chars(next, end_ptr, nsecs);
  if (to_char_res.ec != std::errc() ||
      to_char_res.ptr == end_ptr) {
    assert(false);
    return;
  }
  next = to_char_res.ptr;
  while (next < end_ptr - 1) {
    *next++ = ' ';
  }
  *next++ = '\0';
  {
    std::optional<WithColor> color;
    if (!is_marked) {
      color.emplace(window_, kTimeColorPair);
    }
    mvwaddstr(
        window_.get(),
        row, kTimeStartCol,
        buf.data());
  }
}

void LogWindow::DisplayLevel(
    bool is_marked,
    int row, const LogLevel level) noexcept {
  std::string_view level_str;
  int level_pair = 0;
  switch (level) {
    case LogLevel::Debug:
      level_str = "DEBUG";
      level_pair = kDebugColorPair;
      break;
    case LogLevel::Info:
      level_str = "INFO ";
      level_pair = kInfoColorPair;
      break;
    case LogLevel::Warning:
      level_str = "WARN ";
      level_pair = kWarnColorPair;
      break;
    case LogLevel::Error:
      level_str = "ERROR";
      level_pair = kErrColorPair;
      break;
    default:
      assert(false);
  }
  assert(level_str.size() == kLevelColSize);
  {
    std::optional<WithColor> color;
    if (!is_marked) {
      color.emplace(window_, level_pair);
    }
    mvwaddnstr(
        window_.get(),
        row, kLevelStartCol,
        level_str.data(), level_str.size());
  }
}

void LogWindow::DisplayMessage(
    int row, const std::string_view& message) noexcept {
  if (message_horz_offset_ > message.size()) {
    return;
  }
  std::string_view part_to_display = message.substr(message_horz_offset_);
  mvwaddnstr(
      window_.get(),
      row, kMessageStartCol,
      part_to_display.data(),
      part_to_display.size());
}

void LogWindow::HandleKeyPress(int key) noexcept {
  const std::vector<LogRecord>& records = view_->GetRecords();
  switch (key) {
    case 'm':
      marking_ = !marking_;
      if (marking_) {
        marked_records_begin_ = GetRecordUnderCursor();
        marked_records_end_ = marked_records_begin_ + 1;
        marked_anchor_record_ = marked_records_begin_;
      }
      break;
    case 'j':
    case KEY_DOWN:
      if (cursor_line_ < (num_rows_ - 1)) {
        if (cursor_line_ + first_shown_record_ + 1 < records.size()) {
          ++cursor_line_;
        }
      } else {
        if (GetDisplayedRecordAfterLast() < records.size()) {
          ++first_shown_record_;
        }
      }
      MaybeExtendMarking();
      break;
    case 'k':
    case KEY_UP:
      if (cursor_line_ > 0) {
        --cursor_line_;
      } else {
        if (first_shown_record_ > 0) {
          --first_shown_record_;
        }
      }
      MaybeExtendMarking();
      break;
    case 'h':
    case KEY_LEFT:
      if (message_horz_offset_ > 0) {
        --message_horz_offset_;
      }
      break;
    case 'l':
    case KEY_RIGHT:
      ++message_horz_offset_;
      break;
    case 'G':
      if (records.size() > static_cast<size_t>(num_rows_)) {
        first_shown_record_ = records.size() - num_rows_;
        cursor_line_ = num_rows_ - 1;
      } else {
        cursor_line_ = static_cast<int>(records.size()) - 1;
      }
      break;
  }
}

void LogWindow::MaybeExtendMarking() noexcept {
  if (!marking_) {
    return;
  }
  size_t cur_record = GetRecordUnderCursor();
  if (cur_record < marked_anchor_record_) {
    marked_records_begin_ = cur_record;
    marked_records_end_ = marked_anchor_record_ + 1;
  } else {
    marked_records_begin_ = marked_anchor_record_;
    marked_records_end_ = cur_record + 1;
  }
}

size_t LogWindow::GetDisplayedRecordAfterLast() const noexcept {
  return std::min(
      first_shown_record_ + num_rows_ + 1,
      view_->GetRecords().size());
}

void LogWindow::SetView(LogView* view) noexcept {
  view_ = view;
  first_shown_record_ = 0;
  message_horz_offset_ = 0;
  // TODO(vchigrin): Attempt to preserve scroll position.
  cursor_line_ = 0;
  marked_anchor_record_ = 0;
  // TODO(vchigrin): Attempt to preserve marked region.
  marked_records_begin_ = 0;
  marked_records_end_ = 0;
  marking_ = false;
}

}  // namespace oko
