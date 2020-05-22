// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/log_file_impl.h"
#include "viewer/error_codes.h"

#include <utility>

namespace oko {

LogFileImpl::LogFileImpl(std::filesystem::path file_path) noexcept
    : file_path_(std::move(file_path)) {
}

std::error_code LogFileImpl::Parse() noexcept {
  records_.clear();
  std::error_code ec;
  auto file_size = std::filesystem::file_size(file_path_, ec);
  if (ec) {
    return ec;
  }
  // mapped_file constructor will throw on empty files.
  if (file_size == 0) {
    return ErrorCodes::kOk;
  }
  mapped_file_ = boost::iostreams::mapped_file(
      file_path_, boost::iostreams::mapped_file::readonly);
  if (!mapped_file_.is_open()) {
    return ErrorCodes::kFailedMapFile;
  }
  return ParseImpl(
      std::string_view(mapped_file_.data(), mapped_file_.size()),
      &records_);
}

const std::filesystem::path& LogFileImpl::file_path() const noexcept {
  return file_path_;
}

const std::vector<LogRecord>& LogFileImpl::GetRecords() const noexcept {
  return records_;
}

}  // namespace oko
