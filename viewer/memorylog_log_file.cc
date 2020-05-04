// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/memorylog_log_file.h"

#include <algorithm>
#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <charconv>
#include <utility>

namespace oko {

namespace {
const std::string_view kRecordStartSentinel = "\niPao2ijSahbe0F";
}  //  namespace

bool MemorylogLogFile::Parse(const std::string& file_path) noexcept {
  records_.clear();
  mapped_file_ = boost::iostreams::mapped_file(file_path);
  if (!mapped_file_.is_open()) {
    return false;
  }
  const std::string_view file_data(mapped_file_.data(), mapped_file_.size());
  size_t pos = 0;
  std::vector<RawRecord> raw_records;
  while (pos < file_data.size()) {
    size_t next_entry_start = file_data.find(kRecordStartSentinel, pos);
    if (next_entry_start == std::string_view::npos) {
      break;
    }
    next_entry_start += kRecordStartSentinel.length();
    size_t entry_end = file_data.find_first_of("\0\n", next_entry_start, 2);
    std::string_view entry_data;
    if (entry_end == std::string_view::npos) {
      entry_data = file_data.substr(next_entry_start);
    } else {
      entry_data = file_data.substr(
          next_entry_start, entry_end - next_entry_start);
    }
    pos = entry_end + (file_data[entry_end] != '\n' ? 1 : 0);
    RawRecord raw_rec;
    if (FillRecord(entry_data, &raw_rec)) {
      raw_records.emplace_back(std::move(raw_rec));
    }
  }
  std::sort(
      raw_records.begin(),
      raw_records.end(),
      [](const RawRecord& first, const RawRecord& second) {
        return first.raw_timestamp < second.raw_timestamp;
      });

  // Get timestamps from anchor records and interpolate time on records
  // between them;
  struct AnchorRecordData {
    const LogRecord::time_point time_point;
    const std::vector<RawRecord>::const_iterator it;
  };
  std::vector<AnchorRecordData> anchors;
  for (size_t i = 0, n = raw_records.size(); i < n; ++i) {
    LogRecord::time_point time_point;
    if (ExtractTimestampFromRecord(raw_records[i], &time_point)) {
      anchors.emplace_back(
          AnchorRecordData{time_point, raw_records.cbegin() + i});
    }
  }
  if (anchors.size() < 2) {
    // Can not extrapolate if we have too few anchors.
    return false;
  }
  // Process records before first ahchor, using region between two
  // first anchor points for extrapolation.
  ProcessRecords(
      RawRecordsRange(raw_records.cbegin(), anchors[0].it),
      anchors[0].it->raw_timestamp,
      anchors[0].time_point,
      anchors[1].it->raw_timestamp,
      anchors[1].time_point);
  for (size_t i = 1, n = anchors.size(); i < n; ++i) {
    ProcessRecords(
        RawRecordsRange(anchors[i - 1].it, anchors[i].it),
        anchors[i - 1].it->raw_timestamp,
        anchors[i - 1].time_point,
        anchors[i].it->raw_timestamp,
        anchors[i].time_point);
  }
  // Process records after last anchor, using region between
  // last two anchor points or extrapolation.
  ProcessRecords(
      RawRecordsRange(anchors.back().it, raw_records.cend()),
      anchors[anchors.size() - 2].it->raw_timestamp,
      anchors[anchors.size() - 2].time_point,
      anchors[anchors.size() - 1].it->raw_timestamp,
      anchors[anchors.size() - 1].time_point);
  return true;
}

bool MemorylogLogFile::FillRecord(
    std::string_view entry_data,
    RawRecord* record) noexcept {
  while (!entry_data.empty() && entry_data.front() == ' ') {
    entry_data.remove_prefix(1);
  }
  while (!entry_data.empty() && entry_data.back() == ' ') {
    entry_data.remove_suffix(1);
  }
  size_t first_space = entry_data.find(' ');
  if (first_space == std::string_view::npos) {
    // Corrupted record.
    return false;
  }
  std::string_view stamp = entry_data.substr(0, first_space);
  record->message = entry_data.substr(first_space + 1);
  auto from_chars_res = std::from_chars(
      stamp.data(), stamp.data() + stamp.size(), record->raw_timestamp);
  if (from_chars_res.ec != std::errc() ||
      from_chars_res.ptr != stamp.data() + stamp.size()) {
    // Stamp area seems to be corrupted.
    return false;
  }
  return true;
}

bool MemorylogLogFile::ExtractTimestampFromRecord(
    const RawRecord& raw_rec,
    LogRecord::time_point* result_timestamp) noexcept {
  // Assumes format like
  // "time anchor: 351638.672110347 1588316753.784846326 ...."
  using sv_find_iterator =
      boost::algorithm::split_iterator<std::string_view::iterator>;
  auto range_it = boost::algorithm::make_split_iterator(
      raw_rec.message, boost::algorithm::first_finder(" "));
  static const std::string_view kTime("time");
  static const std::string_view kAnchor("anchor:");
  if (range_it == sv_find_iterator() ||
      !std::equal(kTime.begin(), kTime.end(), range_it->begin())) {
    return false;
  }
  ++range_it;
  if (range_it == sv_find_iterator() ||
      !std::equal(kAnchor.begin(), kAnchor.end(), range_it->begin())) {
    return false;
  }
  ++range_it;
  if (range_it == sv_find_iterator()) {
    return false;
  }
  ++range_it;
  if (range_it == sv_find_iterator()) {
    return false;
  }
  // Now range points to timestamp - seconds.nanoseconds.
  uint64_t seconds = 0, nanoseconds = 0;
  auto from_chars_res = std::from_chars(
      range_it->begin(), range_it->end(), seconds);
  if (from_chars_res.ec != std::errc() ||
      from_chars_res.ptr == range_it->end() ||
      *from_chars_res.ptr != '.') {
    return false;
  }
  const char* ns_string_ptr = from_chars_res.ptr + 1;
  from_chars_res = std::from_chars(
      ns_string_ptr, range_it->end(), nanoseconds);
  if (from_chars_res.ec != std::errc() ||
      from_chars_res.ptr != range_it->end()) {
    return false;
  }
  *result_timestamp = LogRecord::time_point(
      std::chrono::seconds(seconds) + std::chrono::nanoseconds(nanoseconds));
  assert(*result_timestamp != LogRecord::time_point());
  return true;
}

void MemorylogLogFile::ProcessRecords(
    const boost::iterator_range<std::vector<RawRecord>::const_iterator>
        added_region,
    const uint64_t first_raw_time_stamp,
    const LogRecord::time_point first_time_point,
    const uint64_t second_raw_time_stamp,
    const LogRecord::time_point second_time_point) {
  if (added_region.empty()) {
    return;
  }
  const auto tp_duration = second_time_point - first_time_point;
  uint64_t raw_timestamp_duration =
      second_raw_time_stamp - first_raw_time_stamp;
  if (added_region.front().raw_timestamp >= first_raw_time_stamp) {
    // Our region lies after first pivot time point, so we can use duration
    // after first point without risk of overflow.
    for (const RawRecord& raw_rec : added_region) {
      const LogRecord::time_point tp =
           first_time_point + (
               (raw_rec.raw_timestamp - first_raw_time_stamp) * tp_duration) /
                   raw_timestamp_duration;
      records_.emplace_back(
          tp,
          // Memory log does not proviide distinct log levels, so add
          // all records at "info" level.
          LogLevel::Info,
          raw_rec.message);
    }
  } else if (added_region.back().raw_timestamp <= first_raw_time_stamp) {
    // Our region lies before first pivot time point, so we can use duration
    // to first point without risk of overflow.
    for (const RawRecord& raw_rec : added_region) {
      const LogRecord::time_point tp =
           first_time_point - (
               (first_raw_time_stamp - raw_rec.raw_timestamp) * tp_duration) /
                   raw_timestamp_duration;
      records_.emplace_back(
          tp,
          // Memory log does not proviide distinct log levels, so add
          // all records at "info" level.
          LogLevel::Info,
          raw_rec.message);
    }
  } else {
    // first time point must not fall inside added region.
    assert(false);
  }
}

}  // namespace oko
