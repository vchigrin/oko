// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/log_window.h"

#include <algorithm>
#include <array>
#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>
#include <charconv>
#include <chrono>
#include <ctime>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "viewer/ui/color_manager.h"
#include "viewer/ui/ncurses_helpers.h"

namespace oko {

class TimeFormatter {
 public:
  virtual ~TimeFormatter() = default;
  virtual std::string Format(const LogRecord::time_point&) const noexcept = 0;
};

class TimestampTimeFormatter : public TimeFormatter {
 public:
  std::string Format(const LogRecord::time_point& tp) const noexcept override {
    const int secs = std::chrono::duration_cast<std::chrono::seconds>(
        tp.time_since_epoch()).count();
    const int nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        tp.time_since_epoch() - std::chrono::seconds(secs)).count();
    return boost::str(boost::format("%1%.%2$09d") % secs % nsecs);
  }
};

// yyyy-mm-dd hh:mm:ss.ns
class DateTimeTimeFormatter : public TimeFormatter {
 public:
  std::string Format(const LogRecord::time_point& tp) const noexcept override {
    const time_t secs = std::chrono::duration_cast<std::chrono::seconds>(
        tp.time_since_epoch()).count();
    const int nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        tp.time_since_epoch() - std::chrono::seconds(secs)).count();
    std::tm* t = std::localtime(&secs);
    if (!t) {
      assert(false);
      return {};
    }
    return boost::str(boost::format(
        "%1$02d-%2$02d-%3$02d %4$02d:%5$02d:%6$02d.%7$09d") %
            (t->tm_year + 1900) % (t->tm_mon + 1) % t->tm_mday %
            t->tm_hour % t->tm_min % t->tm_sec %
            nsecs);
  }
};

// hh:mm:ss.ns
class TimeOnlyTimeFormatter : public TimeFormatter {
 public:
  std::string Format(const LogRecord::time_point& tp) const noexcept override {
    const time_t secs = std::chrono::duration_cast<std::chrono::seconds>(
        tp.time_since_epoch()).count();
    const int nsecs = std::chrono::duration_cast<std::chrono::nanoseconds>(
        tp.time_since_epoch() - std::chrono::seconds(secs)).count();
    std::tm* t = std::localtime(&secs);
    if (!t) {
      assert(false);
      return {};
    }
    return boost::str(boost::format(
        "%1$02d:%2$02d:%3$02d.%4$09d") %
            t->tm_hour % t->tm_min % t->tm_sec %
            nsecs);
  }
};

static int kTimeFormatsCount = 3;

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
  CreateTimeFormatter();
}

LogWindow::~LogWindow() = default;

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
    wmove(window_.get(), row, 0);

    DisplayTime(is_marked, records[i].timestamp());
    waddch(window_.get(), ' ');

    DisplayLevel(is_marked, records[i].log_level());
    waddch(window_.get(), ' ');

    DisplayMessage(records[i].message());
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
    const LogRecord::time_point time_point) noexcept {
  std::string time_str = current_time_formatter_->Format(time_point);
  std::optional<WithColor> color;
  if (!is_marked) {
    color.emplace(window_, time_color_pair_);
  }
  waddstr(window_.get(), time_str.c_str());
}

void LogWindow::CreateTimeFormatter() noexcept {
  switch (current_time_formatter_number_) {
    case 0:
      current_time_formatter_ = std::make_unique<TimestampTimeFormatter>();
      break;
    case 1:
      current_time_formatter_ = std::make_unique<DateTimeTimeFormatter>();
      break;
    case 2:
      current_time_formatter_ = std::make_unique<TimeOnlyTimeFormatter>();
      break;
    default:
      assert(false);
  }
}

void LogWindow::DisplayLevel(bool is_marked, const LogLevel level) noexcept {
  std::string_view level_str;
  int level_pair = 0;
  // All level strings must have the same size for better appearance on screen.
  static constexpr int kLevelColSize = 5;
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
  (void)kLevelColSize;  // Silence "unused variable" warning in Release mode.
  assert(level_str.size() == kLevelColSize);
  {
    std::optional<WithColor> color;
    if (!is_marked) {
      color.emplace(window_, level_pair);
    }
    waddnstr(
        window_.get(),
        level_str.data(), level_str.size());
  }
}

void LogWindow::DisplayMessage(const std::string_view& message) noexcept {
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
    waddnstr(
        window_.get(),
        part_to_display.data(),
        part_to_display.size());
  } else {
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
    case KEY_F(10):
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
    case 't':
    case KEY_F(12):
      current_time_formatter_number_ = (
          current_time_formatter_number_ + 1) % kTimeFormatsCount;
      CreateTimeFormatter();
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
    const std::vector<std::unique_ptr<LogFilter>>&) noexcept {
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
