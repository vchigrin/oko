// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/log_files_window.h"

#include <algorithm>
#include <array>
#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>
#include <charconv>
#include <future>
#include <memory>
#include <unordered_set>
#include <utility>

#include "viewer/ui/color_manager.h"
#include "viewer/ui/message_window.h"
#include "viewer/ui/progress_window.h"

namespace oko {

LogFilesWindow::LogFilesWindow(
    LogFilesProvider* files_provider,
    int start_row,
    int start_col,
    int num_rows,
    int num_columns)
    : Window(start_row, start_col, num_rows, num_columns),
      files_provider_(files_provider) {
  std::future<outcome::std_result<std::vector<LogFileInfo>>> get_file_infos_async =
      std::async(
        std::launch::async,
        [files_provider]() {
          return files_provider->GetLogFileInfos();
        });
  {
    ProgressWindow progress_window(
        "Retrieving file list...",
        [&get_file_infos_async] {
            return get_file_infos_async.wait_for(
                std::chrono::seconds(0)) == std::future_status::ready;
        });
    progress_window.PostSync();
    Display();
  }
  auto maybe_file_infos = get_file_infos_async.get();
  if (!maybe_file_infos) {
    MessageWindow::PostSync(boost::str(boost::format(
        "Failed retrieve file list. %1%.") %
            maybe_file_infos.error().message()));
    finished_ = true;
    return;
  }
  file_infos_.reserve(maybe_file_infos.value().size());
  file_infos_.insert(
      file_infos_.begin(),
      maybe_file_infos.value().begin(),
      maybe_file_infos.value().end());
  if (file_infos_.empty()) {
    MessageWindow::PostSync("Don't found any log files under specified path");
    finished_ = true;
    return;
  }

  std::sort(
      file_infos_.begin(),
      file_infos_.end(),
      [](const LogFileInfo& first, const LogFileInfo& second) {
        return first.name < second.name;
      });
  ColorManager& cm = ColorManager::instance();
  selected_color_pair_ = cm.RegisterColorPair(COLOR_BLACK, COLOR_WHITE);
  selected_marked_color_pair_ = cm.RegisterColorPair(COLOR_WHITE, COLOR_RED);
  marked_color_pair_ = cm.RegisterColorPair(COLOR_YELLOW, COLOR_RED);
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
    case 'm':
    case KEY_F(10):
      if (!file_infos_.empty()) {
        file_infos_[selected_item_].is_marked =
            !file_infos_[selected_item_].is_marked;
        if (selected_item_ + 1 < file_infos_.size()) {
          SetSelectedItem(selected_item_ + 1);
        }
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
    bool bg_changed = true;
    if (i == selected_item_ && file_infos_[i].is_marked) {
      wbkgdset(window_.get(), COLOR_PAIR(selected_marked_color_pair_));
    } else if (i == selected_item_) {
      wbkgdset(window_.get(), COLOR_PAIR(selected_color_pair_));
    } else if (file_infos_[i].is_marked) {
      wbkgdset(window_.get(), COLOR_PAIR(marked_color_pair_));
    } else {
      bg_changed = false;
    }
    DisplayItem(row, file_infos_[i]);
    if (bg_changed) {
      wbkgdset(window_.get(), original_bkgd);
    }
  }
  wclrtobot(window_.get());
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
  std::vector<outcome::std_result<std::unique_ptr<LogFile>>> maybe_files;
  maybe_files.reserve(file_infos_.size());
  std::future<void> fetch_async =
      std::async(
        std::launch::async,
        [this, &maybe_files] {
          for (size_t i = 0; i < file_infos_.size(); ++i) {
            if (i == selected_item_ || file_infos_[i].is_marked) {
              auto fetch_result = files_provider_->FetchLog(
                  file_infos_[i].name);
              maybe_files.emplace_back(std::move(fetch_result));
            }
          }
        });
  {
    ProgressWindow progress_window(
        "Fetching files...",
        [&fetch_async] {
            return fetch_async.wait_for(
                std::chrono::seconds(0)) == std::future_status::ready;
        });
    progress_window.PostSync();
    Display();
  }
  fetch_async.get();
  std::unordered_set<std::error_code> errors;
  fetched_files_.clear();
  if (!maybe_files.empty()) {
    for (auto& maybe_file : maybe_files) {
      if (!maybe_file) {
        errors.emplace(std::move(maybe_file.error()));
      } else {
        fetched_files_.emplace_back(std::move(maybe_file.value()));
      }
    }
  }
  if (!errors.empty()) {
    std::stringstream message_buf;
    message_buf << "Some files failed to fetch. ";
    for (const auto& error : errors) {
      message_buf << error.message() << ". ";
    }
    MessageWindow::PostSync(message_buf.str());
  }
  finished_ = true;
}

void LogFilesWindow::SearchForFilesBySubstring(std::string str) noexcept {
  if (file_infos_.empty() || str.empty()) {
    return;
  }
  string_to_search_ = std::move(str);
  auto it = std::find_if(
      file_infos_.begin() + selected_item_,
      file_infos_.end(),
      [&s = string_to_search_.value()](const LogFileInfo& r) {
        return r.name.find(s) != std::string::npos;
      });
  if (it != file_infos_.end()) {
    SetSelectedItem(it - file_infos_.begin());
  }
}

void LogFilesWindow::SearchNextEntry() noexcept {
  if (file_infos_.empty() ||
      !string_to_search_ ||
      selected_item_ + 1 >= file_infos_.size()) {
    return;
  }
  auto it = std::find_if(
      file_infos_.begin() + selected_item_ + 1,
      file_infos_.end(),
      [&s = string_to_search_.value()](const LogFileInfo& r) {
        return r.name.find(s) != std::string::npos;
      });
  if (it != file_infos_.end()) {
    SetSelectedItem(it - file_infos_.begin());
  }
}

void LogFilesWindow::SearchPrevEntry() noexcept {
  if (file_infos_.empty() ||
      !string_to_search_ ||
      selected_item_ == 0) {
    return;
  }
  auto it = std::find_if(
      std::make_reverse_iterator(file_infos_.begin() + selected_item_),
      file_infos_.rend(),
      [&s = string_to_search_.value()](const LogFileInfo& r) {
        return r.name.find(s) != std::string::npos;
      });
  if (it != file_infos_.rend()) {
    SetSelectedItem(it.base() - file_infos_.begin() - 1);
  }
}

}  // namespace oko
