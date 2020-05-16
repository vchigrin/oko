// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
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

  bool has_any_file_infos() const noexcept {
    return !file_infos_.empty();
  }

  const std::filesystem::path& fetched_file_path() const noexcept {
    return fetched_file_path_;
  }

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
  std::filesystem::path fetched_file_path_;
  std::vector<LogFileInfo> file_infos_;
};

}  // namespace oko
