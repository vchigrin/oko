// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/log_formats/text_log_file.h"

#include <charconv>
#include <utility>

namespace oko {

// Class for parsing log files in some proprientary project.
bool TextLogFile::Parse(const std::string& file_path) noexcept {
  mapped_file_ = boost::iostreams::mapped_file(file_path);
  nsec_counter_base_ = std::nullopt;
  if (!mapped_file_.is_open()) {
    file_path_ = std::string();
    return false;
  }
  file_path_ = file_path;
  const std::string_view file_data(mapped_file_.data(), mapped_file_.size());
  size_t pos = 0;
  while (pos < file_data.size()) {
    size_t line_end = file_data.find('\n', pos);
    std::string_view next_line = file_data.substr(
        pos, line_end == std::string_view::npos ? line_end : line_end - pos);
    ParseLine(next_line);
    if (line_end == std::string_view::npos) {
      break;
    }
    pos = line_end + 1;
  }
  // Some lines may be misordered, so we must sort.
  std::sort(
      records_.begin(),
      records_.end(),
      [](const LogRecord& first, const LogRecord& second) {
        return first.timestamp() < second.timestamp();
      });
  return true;
}

void TextLogFile::ParseLine(std::string_view line) noexcept {
  // Assumes format:
  // nanoseconds_counter | unused_character seconds.ms YYYY-MM-dd hh:mm:ss time_zone |level | message
  uint64_t nsec_counter = 0;
  auto from_chars_res = std::from_chars(
      line.data(),
      line.data() + line.size(),
      nsec_counter);
  if (from_chars_res.ec != std::errc()) {
    return;
  }
  line.remove_prefix(from_chars_res.ptr - line.data());
  while (!line.empty() && (line[0] == ' ' || line[0] == '|')) {
    line.remove_prefix(1);
  }
  if (line.size() <= 2) {
    return;
  }
  // Skip unused character and space after it
  line.remove_prefix(2);
  uint64_t sec = 0, msec = 0;
  from_chars_res = std::from_chars(
      line.data(),
      line.data() + line.size(),
      sec);
  if (from_chars_res.ec != std::errc() || *from_chars_res.ptr != '.') {
    return;
  }
  line.remove_prefix(from_chars_res.ptr - line.data() + 1);
  // Exactly 3 digits for msec expected.
  if (line.size() < 4 || line[3] != ' ') {
    return;
  }
  from_chars_res = std::from_chars(
      line.data(),
      line.data() + line.size(),
      msec);
  if (from_chars_res.ec != std::errc() ||
      from_chars_res.ptr != line.data() + 3) {
    return;
  }
  line.remove_prefix(from_chars_res.ptr - line.data() + 1);
  size_t next_sep_pos = line.find('|');
  if (next_sep_pos == std::string_view::npos) {
    return;
  }
  line.remove_prefix(next_sep_pos + 1);
  next_sep_pos = line.find('|');
  if (next_sep_pos == std::string_view::npos) {
    return;
  }
  std::string_view level = line.substr(0, next_sep_pos);
  std::string_view message = line.substr(next_sep_pos + 1);
  while (!level.empty() && level[0] == ' ') {
    level.remove_prefix(1);
  }
  while (!level.empty() && level[level.size() - 1] == ' ') {
    level.remove_suffix(1);
  }
  AddRecord(
      sec, msec,
      nsec_counter,
      std::move(level),
      std::move(message));
}

void TextLogFile::AddRecord(
    uint64_t sec, uint64_t msec,
    uint64_t nsec_counter,
    std::string_view level_string,
    std::string_view message) noexcept {
  LogRecord::time_point current_time_point;
  if (!nsec_counter_base_) {
    // Assume first records fit nseconds perfectly, and calculate
    // all subsequent records timestamps based on it.
    current_time_point = LogRecord::time_point {} +
        std::chrono::seconds(sec) + std::chrono::milliseconds(msec);
    nsec_counter_base_ = current_time_point -
        std::chrono::nanoseconds(nsec_counter);
  } else {
    current_time_point = *nsec_counter_base_ +
        std::chrono::nanoseconds(nsec_counter);
  }
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
    return;
  }
  records_.emplace_back(current_time_point, level, std::move(message));
}

}  // namespace oko
