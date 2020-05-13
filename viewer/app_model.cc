// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/app_model.h"

#include <algorithm>
#include <cassert>
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
  // TODO(vchigrin): Attempt to preserve scroll position.
  selected_record_ = 0;
  sig_filter_set_changed_(active_filters_);
}

void AppModel::SetMarkedRegion(
    size_t marked_records_begin, size_t marked_records_end) noexcept {
  marked_records_begin_ = marked_records_begin;
  marked_records_end_ = marked_records_end;
}

void AppModel::SetSelectedRecord(size_t index) noexcept {
  assert(index < active_view().GetRecords().size());
  selected_record_ = index;
  sig_selected_record_changed_(index);
}

void AppModel::TrySelectNextRecord() noexcept {
  const auto& records = active_view().GetRecords();
  if (selected_record_ + 1 < records.size()) {
    SetSelectedRecord(selected_record_ + 1);
  }
}

void AppModel::TrySelectPrevRecord() noexcept {
  if (selected_record_ > 0) {
    SetSelectedRecord(selected_record_ - 1);
  }
}

LogRecord::time_point AppModel::GetSelectedRecordTimestamp() const noexcept {
  const auto& records = active_view().GetRecords();
  if (records.empty()) {
    return LogRecord::time_point{};
  }
  return records[selected_record_].timestamp();
}

void AppModel::SelectRecordByTimestamp(LogRecord::time_point tp) noexcept {
  const auto& records = active_view().GetRecords();
  if (records.empty()) {
    return;
  }
  auto it = std::lower_bound(
      records.begin(),
      records.end(),
      tp,
      [](const LogRecord& first, const LogRecord::time_point second) {
        return first.timestamp() < second;
      });
  if (it == records.end()) {
    // Out of range - select last record.
    --it;
  } else {
    if (it != records.begin() && it->timestamp() > tp) {
      // Select last record with timestamp less then or equal to provided
      // by user.
      --it;
    }
  }
  SetSelectedRecord(it - records.begin());
}

void AppModel::SearchForMessage(std::string text) noexcept {
  search_text_ = std::move(text);
  const auto& records = active_view().GetRecords();
  if (records.empty() || search_text_.empty()) {
    return;
  }
  auto it = std::find_if(
      records.begin() + selected_record_,
      records.end(),
      [&txt = search_text_](const LogRecord& r) {
        return r.message().find(txt) != std::string_view::npos;
      });
  if (it != records.end()) {
    SetSelectedRecord(it - records.begin());
  }
}

void AppModel::SearchNextEntry() noexcept {
  const auto& records = active_view().GetRecords();
  if (records.empty() || search_text_.empty()) {
    return;
  }
  if (selected_record_ + 1 >= records.size()) {
    // Already at last record.
    return;
  }

  auto it = std::find_if(
      records.begin() + selected_record_ + 1,
      records.end(),
      [&txt = search_text_](const LogRecord& r) {
        return r.message().find(txt) != std::string_view::npos;
      });
  if (it != records.end()) {
    SetSelectedRecord(it - records.begin());
  }
}

void AppModel::SearchPrevEntry() noexcept {
  const auto& records = active_view().GetRecords();
  if (records.empty() || search_text_.empty()) {
    return;
  }
  if (selected_record_ == 0) {
    // Already at first record.
    return;
  }

  auto it = std::find_if(
      std::make_reverse_iterator(records.begin() + selected_record_),
      records.rend(),
      [&txt = search_text_](const LogRecord& r) {
        return r.message().find(txt) != std::string_view::npos;
      });
  if (it != records.rend()) {
    SetSelectedRecord(it.base() - records.begin() - 1);
  }
}

}  // namespace oko
