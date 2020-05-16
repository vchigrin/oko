// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/log_files_window.h"

#include <algorithm>
#include <array>
#include <boost/algorithm/string/replace.hpp>
#include <charconv>

#include "viewer/ui/color_manager.h"

namespace oko {

namespace {

std::regex FileMaskToRegEx(std::string mask) noexcept {
  std::vector<char> result{mask.begin(), mask.end()};
  boost::algorithm::replace_all(result, ".", "\\.");
  boost::algorithm::replace_all(result, "^", "\\^");
  boost::algorithm::replace_all(result, "$", "\\$");
  boost::algorithm::replace_all(result, "*", ".*");
  boost::algorithm::replace_all(result, "?", ".");
  result.insert(result.begin(), '^');
  result.push_back('$');
  result.push_back(0);
  return std::regex(result.data());
}

}  // namespace

LogFilesWindow::LogFilesWindow(
    LogFilesProvider* files_provider,
    int start_row,
    int start_col,
    int num_rows,
    int num_columns)
    : Window(start_row, start_col, num_rows, num_columns),
      files_provider_(files_provider),
      file_infos_(files_provider_->GetLogFileInfos()) {
  std::sort(
      file_infos_.begin(),
      file_infos_.end(),
      [](const LogFileInfo& first, const LogFileInfo& second) {
        return first.name < second.name;
      });
  ColorManager& cm = ColorManager::instance();
  selected_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_WHITE);
}

void LogFilesWindow::HandleKeyPress(int key) noexcept {
  switch (key) {
    case 'j':
    case KEY_DOWN:
      if (selected_item_ + 1 < file_infos_.size()) {
        SetSelectedItem(selected_item_ + 1);
      }
      break;
    case 'k':
    case KEY_UP:
      if (selected_item_ > 0) {
        SetSelectedItem(selected_item_ - 1);
      }
      break;
    case KEY_ENTER:
    case '\n':
      Finish();
      break;
  }
}

void LogFilesWindow::DisplayImpl() noexcept {
  DisplayTitle();
  if (num_rows_ == 1) {
    return;
  }
  const size_t limit = std::min(
      file_infos_.size(),
      first_shown_item_ + num_rows_ - 1);
  int original_bkgd = getbkgd(window_.get());
  for (size_t i = first_shown_item_, row = 1; i < limit; ++i, ++row) {
    if (i == selected_item_) {
      wbkgdset(window_.get(), COLOR_PAIR(selected_color_pair_));
    }
    DisplayItem(row, file_infos_[i]);
    if (i == selected_item_) {
      wbkgdset(window_.get(), original_bkgd);
    }
  }
}

void LogFilesWindow::DisplayTitle() noexcept {
  mvwhline(window_.get(), 0, 0, 0, num_columns_);
  mvwaddstr(window_.get(), 0, 1, "Log file name");
  const std::string_view kFileSize{"File size"};
  if (num_columns_ > kFileSize.size() + 1) {
    mvwaddstr(
        window_.get(),
        0,
        num_columns_ - kFileSize.size() - 1,
        kFileSize.data());
  }
}

void LogFilesWindow::DisplayItem(int row, const LogFileInfo& info) noexcept {
  mvwaddstr(window_.get(), row, 0, info.name.c_str());
  wclrtoeol(window_.get());
  std::array<char, 22> buf;
  buf[0] = ' ';
  auto res = std::to_chars(
      buf.data() + 1,
      buf.data() + buf.size(),
      info.size);
  if (res.ec != std::errc()) {
    assert(false);
    return;
  }
  *res.ptr = 0;
  mvwaddstr(window_.get(),
      row, num_columns_ - std::strlen(buf.data()), buf.data());
}

void LogFilesWindow::SetSelectedItem(size_t new_item) noexcept {
  selected_item_ = new_item;
  const int list_height = num_rows_ - 1;
  if (selected_item_ >= (first_shown_item_ + list_height)) {
    first_shown_item_ = std::max<int>(0, selected_item_ - list_height + 1);
  }
  if (selected_item_ < first_shown_item_) {
    first_shown_item_ = selected_item_;
  }
}

void LogFilesWindow::Finish() noexcept {
  fetched_file_path_ = files_provider_->FetchLog(
     file_infos_[selected_item_].name);
  finished_ = true;
}

void LogFilesWindow::SearchForFilesByMask(std::string mask) noexcept {
  if (file_infos_.empty() || mask.empty()) {
    return;
  }
  regex_to_search_ = FileMaskToRegEx(mask);
  auto it = std::find_if(
      file_infos_.begin() + selected_item_,
      file_infos_.end(),
      [&re = regex_to_search_.value()](const LogFileInfo& r) {
        return std::regex_match(r.name, re);
      });
  if (it != file_infos_.end()) {
    SetSelectedItem(it - file_infos_.begin());
  }
}

void LogFilesWindow::SearchNextEntry() noexcept {
  if (file_infos_.empty() ||
      !regex_to_search_ ||
      selected_item_ + 1 >= file_infos_.size()) {
    return;
  }
  auto it = std::find_if(
      file_infos_.begin() + selected_item_ + 1,
      file_infos_.end(),
      [&re = regex_to_search_.value()](const LogFileInfo& r) {
        return std::regex_match(r.name, re);
      });
  if (it != file_infos_.end()) {
    SetSelectedItem(it - file_infos_.begin());
  }
}

void LogFilesWindow::SearchPrevEntry() noexcept {
  if (file_infos_.empty() ||
      !regex_to_search_ ||
      selected_item_ == 0) {
    return;
  }
  auto it = std::find_if(
      std::make_reverse_iterator(file_infos_.begin() + selected_item_),
      file_infos_.rend(),
      [&re = regex_to_search_.value()](const LogFileInfo& r) {
        return std::regex_match(r.name, re);
      });
  if (it != file_infos_.rend()) {
    SetSelectedItem(it.base() - file_infos_.begin() - 1);
  }
}

}  // namespace oko
