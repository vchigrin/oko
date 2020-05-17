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
  FilterSetChangeInfo info = BeforeFilterSetChanged();
  active_filters_.push_back(new_filter);
  AfterFilterSetChanged(std::move(info));
}

void AppModel::RemoveAllFilters() noexcept {
  if (active_filters_.empty()) {
    return;
  }
  FilterSetChangeInfo info = BeforeFilterSetChanged();
  for (auto* filter : active_filters_) {
    delete filter;
  }
  active_filters_.clear();
  AfterFilterSetChanged(std::move(info));
}

void AppModel::RemoveLastFilter() noexcept {
  if (active_filters_.empty()) {
    return;
  }
  FilterSetChangeInfo info = BeforeFilterSetChanged();
  delete active_filters_.back();
  active_filters_.pop_back();
  AfterFilterSetChanged(std::move(info));
}

AppModel::FilterSetChangeInfo AppModel::BeforeFilterSetChanged() noexcept {
  const auto& records = active_view().GetRecords();
  if (records.empty()) {
    return {};
  }
  AppModel::FilterSetChangeInfo result;
  if (marked_records_begin_ != marked_records_end_) {
    result.first_marked_record_ts = records[marked_records_begin_].timestamp();
    result.last_marked_record_ts = records[
        marked_records_end_ - 1].timestamp();
  }
  result.selected_record_ts = records[selected_record_].timestamp();
  return result;
}

void AppModel::AfterFilterSetChanged(FilterSetChangeInfo info) noexcept {
  sig_filter_set_changed_(active_filters_);
  const auto& records = active_view().GetRecords();
  if (records.empty()) {
    return;
  }
  SelectRecordByTimestamp(info.selected_record_ts);
  if (info.first_marked_record_ts && info.last_marked_record_ts) {
    marked_records_begin_ = GetIndexTimestampLessThenOrEqual(
        *info.first_marked_record_ts);
    if (records[marked_records_begin_].timestamp() <
          *info.first_marked_record_ts) {
      // First marked record in region disappeared.
      // Make region smaller, not wider.
      ++marked_records_begin_;
    }
    marked_records_end_ = GetIndexTimestampLessThenOrEqual(
        *info.last_marked_record_ts);
    if (marked_records_end_ < records.size()) {
      ++marked_records_end_;
    }
    if (marked_records_end_ < marked_records_begin_) {
      // Clear marked region - it entirely disappeared.
      marked_records_end_ = marked_records_begin_;
    }
  }
}

void AppModel::SetMarkedRegion(
    size_t marked_records_begin, size_t marked_records_end) noexcept {
  marked_records_begin_ = marked_records_begin;
  marked_records_end_ = marked_records_end;
}

void AppModel::SetSelectedRecord(size_t index) noexcept {
  const auto& records = active_view().GetRecords();
  if (records.empty()) {
    return;
  }
  assert(index < records.size());
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

size_t AppModel::GetIndexTimestampLessThenOrEqual(
    LogRecord::time_point tp) noexcept {
  const auto& records = active_view().GetRecords();
  if (records.empty()) {
    return 0;
  }
  auto it = std::lower_bound(
      records.begin(),
      records.end(),
      tp,
      [](const LogRecord& first, const LogRecord::time_point second) {
        return first.timestamp() < second;
      });
  if (it == records.end()) {
    // Out of range - return last record.
    return records.size() - 1;
  } else {
    if (it != records.begin() && it->timestamp() > tp) {
      // Select last record with timestamp less then or equal to provided
      // by user.
      --it;
    }
  }
  return it - records.begin();
}

void AppModel::SelectRecordByTimestamp(LogRecord::time_point tp) noexcept {
  SetSelectedRecord(GetIndexTimestampLessThenOrEqual(std::move(tp)));
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
