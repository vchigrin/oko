// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <memory>
#include <string>
#include <vector>

#include <boost/signals2/signal.hpp>

#include "viewer/log_file.h"
#include "viewer/log_pattern_filter.h"

namespace oko {

class AppModel {
 public:
  explicit AppModel(std::unique_ptr<oko::LogFile> file);
  ~AppModel();

  LogView* active_view() noexcept {
    if (active_filters_.empty()) {
      return file_.get();
    } else {
      return active_filters_.back();
    }
  }

  void AppendFilter(
      std::string pattern,
      bool is_include_filter) noexcept;
  void RemoveAllFilters() noexcept;
  void RemoveLastFilter() noexcept;

  const std::vector<oko::LogPatternFilter*>&
      active_filters() const noexcept {
    return active_filters_;
  }

  const std::string& file_path() const noexcept {
    return file_->file_path();
  }

  using FilterSetChangedSignature = void (
      const std::vector<oko::LogPatternFilter*>&);

  boost::signals2::connection ConnectFilterSetChanged(
      boost::signals2::slot<FilterSetChangedSignature> slot) noexcept {
    return sig_filter_set_changed_.connect(slot);
  }

 private:
  std::unique_ptr<LogFile> file_;
  std::vector<LogPatternFilter*> active_filters_;

  boost::signals2::signal<FilterSetChangedSignature> sig_filter_set_changed_;
};

}  // namespace oko
