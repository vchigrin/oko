// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <algorithm>
#include <optional>
#include <memory>
#include <string>
#include <vector>

#include <boost/signals2/signal.hpp>

#include "viewer/log_file.h"
#include "viewer/log_filter.h"
#include "viewer/merged_log_view.h"

namespace oko {

class AppModel {
 public:
  explicit AppModel(std::vector<std::unique_ptr<oko::LogFile>> files);
  ~AppModel();

  const LogView& active_view() const noexcept {
    if (!active_filters_.empty()) {
      return *active_filters_.back();
    } else if (merger_) {
      return *merger_;
    } else {
      return *files_[0];
    }
  }

  size_t unfiltered_records_count() const noexcept {
    if (merger_) {
      return merger_->GetRecords().size();
    } else {
      return files_[0]->GetRecords().size();
    }
  }

  size_t marked_records_count() const noexcept {
    return marked_records_end_ - marked_records_begin_;
  }

  size_t filtered_records_count() const noexcept {
    return active_view().GetRecords().size();
  }

  void AppendFilter(std::unique_ptr<LogFilter> new_filter) noexcept;
  void RemoveAllFilters() noexcept;
  void RemoveLastFilter() noexcept;

  const std::vector<std::unique_ptr<oko::LogFilter>>&
      active_filters() const noexcept {
    return active_filters_;
  }

  std::vector<std::filesystem::path> GetFilePaths() const noexcept {
    std::vector<std::filesystem::path> result;
    result.reserve(files_.size());
    std::transform(
        files_.begin(),
        files_.end(),
        std::back_inserter(result),
        [](const std::unique_ptr<LogFile>& f) {
          return f->file_path();
        });
    return result;
  }

  void SetMarkedRegion(
      size_t marked_records_begin, size_t marked_records_end) noexcept;

  bool IsMarked(size_t index) const noexcept {
    return index >= marked_records_begin_ && index < marked_records_end_;
  }

  void SetSelectedRecord(size_t index) noexcept;
  void TrySelectNextRecord() noexcept;
  void TrySelectPrevRecord() noexcept;

  void SearchForMessage(std::string text) noexcept;
  void SearchNextEntry() noexcept;
  void SearchPrevEntry() noexcept;

  const std::string& search_text() const noexcept {
    return search_text_;
  }

  size_t selected_record() const noexcept {
    return selected_record_;
  }

  LogRecord::time_point::duration marked_duration() const noexcept {
    if (marked_records_end_ == marked_records_begin_) {
      return {};
    }
    const auto& records = active_view().GetRecords();
    return records[marked_records_end_ - 1].timestamp() -
        records[marked_records_begin_].timestamp();
  }

  using FilterSetChangedSignature = void (
      const std::vector<std::unique_ptr<LogFilter>>&);
  // Argument is new |selected_record()| value.
  using SelectedRecordChangedSignature = void (size_t);

  boost::signals2::connection ConnectFilterSetChanged(
      boost::signals2::slot<FilterSetChangedSignature> slot) noexcept {
    return sig_filter_set_changed_.connect(slot);
  }
  boost::signals2::connection ConnectSelectedRecordChanged(
      boost::signals2::slot<SelectedRecordChangedSignature> slot) noexcept {
    return sig_selected_record_changed_.connect(slot);
  }

  // Select last record with timestamp less then or equal to |tp|.
  void SelectRecordByTimestamp(LogRecord::time_point tp) noexcept;
  LogRecord::time_point GetSelectedRecordTimestamp() const noexcept;

 private:
  struct FilterSetChangeInfo {
    LogRecord::time_point selected_record_ts;
    std::optional<LogRecord::time_point> first_marked_record_ts;
    std::optional<LogRecord::time_point> last_marked_record_ts;
  };
  FilterSetChangeInfo BeforeFilterSetChanged() noexcept;
  void AfterFilterSetChanged(FilterSetChangeInfo info) noexcept;
  size_t GetIndexTimestampLessThenOrEqual(LogRecord::time_point tp) noexcept;

  const std::vector<std::unique_ptr<LogFile>> files_;
  //  May be nullptr if |files_| have only one item.
  std::unique_ptr<MergedLogView> merger_;
  std::vector<std::unique_ptr<LogFilter>> active_filters_;
  // Index of first marked record, inclusively.
  size_t marked_records_begin_ = 0;
  // One plus index of last marked record.
  size_t marked_records_end_ = 0;
  size_t selected_record_ = 0;

  std::string search_text_;

  boost::signals2::signal<FilterSetChangedSignature> sig_filter_set_changed_;
  boost::signals2::signal<SelectedRecordChangedSignature>
      sig_selected_record_changed_;
};

}  // namespace oko
