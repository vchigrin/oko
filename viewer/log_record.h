// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <chrono>
#include <string_view>


namespace oko {

enum class LogLevel {
  Debug,
  Info,
  Warning,
  Error,
};

class LogRecord {
 public:
  using time_point = std::chrono::time_point<
      std::chrono::system_clock,
      std::chrono::nanoseconds>;

  LogRecord(
      const time_point& timestamp,
      LogLevel log_level,
      std::string_view message) noexcept
     : timestamp_(timestamp),
       log_level_(log_level),
       message_(message) {}

  LogLevel log_level() const noexcept {
    return log_level_;
  }

  const std::string_view message() const noexcept {
    return message_;
  }

  const time_point& timestamp() const noexcept {
    return timestamp_;
  }

 private:
  const time_point timestamp_;
  const LogLevel log_level_;
  const std::string_view message_;
};


}  // namespace oko
