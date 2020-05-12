// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/app_model.h"

#include <utility>

namespace oko {

AppModel::AppModel(std::unique_ptr<oko::LogFile> file)
    : file_(std::move(file)) {
}

AppModel::~AppModel() {
  for (auto* filter : active_filters_) {
    delete filter;
  }
}

void AppModel::AppendFilter(
    std::string pattern,
    bool is_include_filter) noexcept {
  oko::LogPatternFilter* new_filter = new oko::LogPatternFilter(
      &active_view(),
      std::move(pattern),
      is_include_filter);
  active_filters_.push_back(new_filter);
  FilterSetChanged();
}

void AppModel::RemoveAllFilters() noexcept {
  if (active_filters_.empty()) {
    return;
  }
  active_filters_.clear();
  FilterSetChanged();
}

void AppModel::RemoveLastFilter() noexcept {
  if (active_filters_.empty()) {
    return;
  }
  delete active_filters_.back();
  active_filters_.pop_back();
  FilterSetChanged();
}

void AppModel::FilterSetChanged() noexcept {
  // TODO(vchigrin): Attempt to preserve marked region.
  marked_records_begin_ = 0;
  marked_records_end_ = 0;
  sig_filter_set_changed_(active_filters_);
}

void AppModel::SetMarkedRegion(
    size_t marked_records_begin, size_t marked_records_end) noexcept {
  marked_records_begin_ = marked_records_begin;
  marked_records_end_ = marked_records_end;
}

}  // namespace oko
