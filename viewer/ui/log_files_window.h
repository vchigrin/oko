// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "viewer/log_files_provider.h"
#include "viewer/ui/window.h"

namespace oko {

class LogFilesWindow : public Window {
 public:
  LogFilesWindow(
      LogFilesProvider* files_provider,
      int start_row,
      int start_col,
      int num_rows,
      int num_columns);

  void HandleKeyPress(int key) noexcept override;

  bool finished() const noexcept {
    return finished_;
  }

  std::vector<std::unique_ptr<LogFile>> RetrieveFetchedFiles() noexcept {
    return std::move(fetched_files_);
  }

  void SearchForFilesByMask(std::string mask) noexcept;
  void SearchNextEntry() noexcept;
  void SearchPrevEntry() noexcept;

 protected:
  void DisplayImpl() noexcept override;
  void DisplayTitle() noexcept;
  void SetSelectedItem(size_t new_item) noexcept;
  void DisplayItem(int row, const LogFileInfo& info) noexcept;
  void Finish() noexcept;

  LogFilesProvider* files_provider_;
  bool finished_ = false;
  size_t selected_item_ = 0;
  size_t first_shown_item_ = 0;
  int selected_color_pair_ = 0;
  int selected_marked_color_pair_ = 0;
  int marked_color_pair_ = 0;
  std::vector<std::unique_ptr<LogFile>> fetched_files_;

  struct LogFileInfoAndMark : public LogFileInfo {
    LogFileInfoAndMark(LogFileInfo& second)
        : LogFileInfo(second) {}

    bool is_marked = false;
  };
  std::vector<LogFileInfoAndMark> file_infos_;
  std::optional<std::regex> regex_to_search_;
};

}  // namespace oko
