// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/log_formats/text_log_file.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <charconv>
#include <utility>

namespace oko {

namespace {

template<typename Predicate>
inline void TrimLeft(std::string_view& str, Predicate p) noexcept {
  while (!str.empty() && p(str[0])) {
    str.remove_prefix(1);
  }
}

template<typename Predicate>
inline void Trim(std::string_view& str, Predicate p) noexcept {
  while (!str.empty() && p(str[0])) {
    str.remove_prefix(1);
  }
  while (!str.empty() && p(str.back())) {
    str.remove_suffix(1);
  }
}

}  // namespace

// Class for parsing log files in some proprientary project.
void TextLogFile::ParseImpl(
    std::string_view file_data,
    std::vector<LogRecord>* records) noexcept {
  nsec_counter_base_ = std::nullopt;
  size_t pos = 0;
  std::optional<RawRecordInfo> pending_record;
  while (pos < file_data.size()) {
    size_t line_end = file_data.find('\n', pos);
    std::string_view next_line = file_data.substr(
        pos, line_end == std::string_view::npos ? line_end : line_end - pos);

    RawRecordInfo next_record;
    if (ParseLine(next_line, next_record)) {
      if (pending_record) {
        AddRecord(records, pending_record.value());
      }
      // Preserve record, since next line after it may be sticked with
      // this record message.
      pending_record = next_record;
    } else {
      // Failed parse current line - extend pending record message.
      if (pending_record) {
        const char* next_end = next_line.data() + next_line.size();
        assert(next_end >= pending_record->message.data());
        pending_record->message = std::string_view(
            pending_record->message.data(),
            next_end - pending_record->message.data());
      }
    }
    if (line_end == std::string_view::npos) {
      break;
    }
    pos = line_end + 1;
  }
  if (pending_record) {
    AddRecord(records, pending_record.value());
  }
  // Some lines may be misordered, so we must sort.
  std::sort(
      records->begin(),
      records->end(),
      [](const LogRecord& first, const LogRecord& second) {
        return first.timestamp() < second.timestamp();
      });
}

bool TextLogFile::ParseLine(
    std::string_view line,
    TextLogFile::RawRecordInfo& info) noexcept {
  // Assumes format:
  // nanoseconds_counter | unused_character seconds.ms YYYY-MM-dd hh:mm:ss time_zone |level | message
  info.nsec_counter = 0;
  auto from_chars_res = std::from_chars(
      line.data(),
      line.data() + line.size(),
      info.nsec_counter);
  if (from_chars_res.ec != std::errc()) {
    return false;
  }
  line.remove_prefix(from_chars_res.ptr - line.data());
  TrimLeft(line, boost::algorithm::is_any_of(" |"));
  if (line.size() <= 2) {
    return false;
  }
  // Skip unused character and space after it
  line.remove_prefix(2);
  from_chars_res = std::from_chars(
      line.data(),
      line.data() + line.size(),
      info.sec);
  if (from_chars_res.ec != std::errc() || *from_chars_res.ptr != '.') {
    return false;
  }
  line.remove_prefix(from_chars_res.ptr - line.data() + 1);
  // Exactly 3 digits for msec expected.
  if (line.size() < 4 || line[3] != ' ') {
    return false;
  }
  from_chars_res = std::from_chars(
      line.data(),
      line.data() + line.size(),
      info.msec);
  if (from_chars_res.ec != std::errc() ||
      from_chars_res.ptr != line.data() + 3) {
    return false;
  }
  line.remove_prefix(from_chars_res.ptr - line.data() + 1);
  size_t next_sep_pos = line.find('|');
  if (next_sep_pos == std::string_view::npos) {
    return false;
  }
  line.remove_prefix(next_sep_pos + 1);
  next_sep_pos = line.find('|');
  if (next_sep_pos == std::string_view::npos) {
    return false;
  }
  std::string_view level_string = line.substr(0, next_sep_pos);
  info.message = line.substr(next_sep_pos + 1);
  Trim(level_string, boost::is_any_of(" "));

  struct LevelLabelAndValue {
    const std::string_view label;
    const LogLevel value;
  };
  // TODO(vchigrin): Consider supporting all log levels in log viewer.
  static const LevelLabelAndValue kLevels[] = {
      {"MORE", LogLevel::Debug},
      {"DEBUG", LogLevel::Debug},
      {"TRACE", LogLevel::Debug},
      {"INFO", LogLevel::Info},
      {"WARN", LogLevel::Warning},
      {"ERROR", LogLevel::Error},
      {"CRIT", LogLevel::Error},
      {"ALERT", LogLevel::Error},
      {"EMERG", LogLevel::Error},
      {"FATAL", LogLevel::Error},
  };
  LogLevel level = LogLevel::Invalid;
  for (const auto& label_and_val : kLevels) {
    if (level_string == label_and_val.label) {
      level = label_and_val.value;
      break;
    }
  }
  if (level == LogLevel::Invalid) {
    return false;
  }
  info.level=level;
  return true;
}

void TextLogFile::AddRecord(
    std::vector<LogRecord>* records,
    const RawRecordInfo& info) noexcept {
  LogRecord::time_point current_time_point;
  if (!nsec_counter_base_) {
    // Assume first records fit nseconds perfectly, and calculate
    // all subsequent records timestamps based on it.
    current_time_point = LogRecord::time_point {} +
        std::chrono::seconds(info.sec) + std::chrono::milliseconds(info.msec);
    nsec_counter_base_ = current_time_point -
        std::chrono::nanoseconds(info.nsec_counter);
  } else {
    current_time_point = *nsec_counter_base_ +
        std::chrono::nanoseconds(info.nsec_counter);
  }
  std::string_view msg = info.message;
  Trim(msg, boost::is_any_of(" \n\r"));
  records->emplace_back(current_time_point, info.level, msg);
}

// static
bool TextLogFile::NameMatches(const std::string& file_name) noexcept {
  return boost::ends_with(file_name, "-async-stdout.log") ||
         boost::ends_with(file_name, "-async-stderr.log");
}

}  // namespace oko
