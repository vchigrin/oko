// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/log_window.h"

#include <algorithm>
#include <array>
#include <boost/algorithm/string/replace.hpp>
#include <charconv>
#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "viewer/ui/color_manager.h"
#include "viewer/ui/ncurses_helpers.h"

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
    AppModel* model,
    int start_row,
    int start_col,
    int num_rows,
    int num_columns) noexcept
    : Window(start_row, start_col, num_rows, num_columns),
      view_(&model->active_view()),
      app_model_(model) {
  ColorManager& cm = ColorManager::instance();
  time_color_pair_ = cm.RegisterColorPair(COLOR_YELLOW, COLOR_BLACK);
  debug_color_pair_ = cm.RegisterColorPair(COLOR_BLUE, COLOR_BLACK);
  info_color_pair_ = cm.RegisterColorPair(COLOR_GREEN, COLOR_BLACK);
  warn_color_pair_ = cm.RegisterColorPair(COLOR_YELLOW, COLOR_BLACK);
  err_color_pair_ = cm.RegisterColorPair(COLOR_RED, COLOR_BLACK);
  mark_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_RED);
  search_text_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_YELLOW);

  filter_set_changed_conn_ =
      app_model_->ConnectFilterSetChanged(
          std::bind(
              &LogWindow::FilterSetChanged,
              this,
              std::placeholders::_1));

  selected_record_changed_conn_ =
      app_model_->ConnectSelectedRecordChanged(
          std::bind(
              &LogWindow::SelectedRecordChanged,
              this,
              std::placeholders::_1));
}

void LogWindow::DisplayImpl() noexcept {
  const size_t after_last_record = GetDisplayedRecordAfterLast();
  const std::vector<LogRecord>& records = view_->GetRecords();
  const size_t selected_record = app_model_->selected_record();
  for (size_t i = first_shown_record_, row = 0;
      i < after_last_record; ++i, ++row) {
    if (i == selected_record) {
      wattron(window_.get(), A_REVERSE);
    }
    const bool is_marked = app_model_->IsMarked(i);
    if (is_marked) {
      wattron(window_.get(), COLOR_PAIR(mark_color_pair_));
    }
    DisplayTime(is_marked, row, records[i].timestamp());
    mvwaddch(window_.get(), row, kTimeStartCol + kTimeColSize, ' ');
    DisplayLevel(is_marked, row, records[i].log_level());
    mvwaddch(window_.get(), row, kLevelStartCol + kLevelColSize, ' ');
    DisplayMessage(row, records[i].message());
    wclrtoeol(window_.get());
    if (is_marked) {
      wattroff(window_.get(), COLOR_PAIR(mark_color_pair_));
    }
    if (i == selected_record) {
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
  // Ensure exactly 9 digits will be for nsecs, add zeros as required.
  uint64_t margin = 100000000;
  while (nsecs < margin) {
    *next++ = '0';
    margin /= 10;
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
      color.emplace(window_, time_color_pair_);
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
      level_pair = debug_color_pair_;
      break;
    case LogLevel::Info:
      level_str = "INFO ";
      level_pair = info_color_pair_;
      break;
    case LogLevel::Warning:
      level_str = "WARN ";
      level_pair = warn_color_pair_;
      break;
    case LogLevel::Error:
      level_str = "ERROR";
      level_pair = err_color_pair_;
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
  std::string part_to_display(
      message.data() + message_horz_offset_,
      message.data() + message.size());
  part_to_display = boost::algorithm::replace_all_copy(
      part_to_display, "\n", "\\n");
  const std::string& search_text = app_model_->search_text();
  size_t search_index = part_to_display.find(search_text);
  if (search_text.empty() || search_index == std::string_view::npos) {
    mvwaddnstr(
        window_.get(),
        row, kMessageStartCol,
        part_to_display.data(),
        part_to_display.size());
  } else {
    wmove(window_.get(), row, kMessageStartCol);
    while (!part_to_display.empty()) {
      waddnstr(
          window_.get(),
          part_to_display.data(),
          search_index);
      if (search_index != std::string_view::npos) {
        WithColor colorer(window_, search_text_color_pair_);
        waddnstr(
            window_.get(),
            part_to_display.data() + search_index,
            search_text.size());
        part_to_display = part_to_display.substr(
            search_index + search_text.size());
        search_index = part_to_display.find(search_text);
      } else {
        break;
      }
    }
  }
}

void LogWindow::HandleKeyPress(int key) noexcept {
  switch (key) {
    case 'm':
      marking_ = !marking_;
      if (marking_) {
        const size_t cur_record = app_model_->selected_record();
        app_model_->SetMarkedRegion(cur_record, cur_record + 1);
        marked_anchor_record_ = cur_record;
      }
      break;
    case 'j':
    case KEY_DOWN:
      app_model_->TrySelectNextRecord();
      break;
    case 'k':
    case KEY_UP:
      app_model_->TrySelectPrevRecord();
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
  }
}

void LogWindow::MaybeExtendMarking() noexcept {
  if (!marking_) {
    return;
  }
  size_t cur_record = app_model_->selected_record();
  if (cur_record < marked_anchor_record_) {
    app_model_->SetMarkedRegion(cur_record, marked_anchor_record_ + 1);
  } else {
    app_model_->SetMarkedRegion(marked_anchor_record_, cur_record + 1);
  }
}

size_t LogWindow::GetDisplayedRecordAfterLast() const noexcept {
  return std::min(
      first_shown_record_ + num_rows_,
      view_->GetRecords().size());
}

void LogWindow::FilterSetChanged(
    const std::vector<LogPatternFilter*>&) noexcept {
  view_ = &app_model_->active_view();
  first_shown_record_ = 0;
  message_horz_offset_ = 0;
  marked_anchor_record_ = 0;
  marking_ = false;
}

void LogWindow::SelectedRecordChanged(size_t selected_record) noexcept {
  if (selected_record >= GetDisplayedRecordAfterLast()) {
    first_shown_record_ = std::max<int>(0, selected_record - num_rows_ + 1);
  }
  if (selected_record < first_shown_record_) {
    first_shown_record_ = selected_record;
  }
  MaybeExtendMarking();
}

}  // namespace oko
