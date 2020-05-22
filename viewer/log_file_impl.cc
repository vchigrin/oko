// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/log_file_impl.h"
#include "viewer/error_codes.h"

namespace oko {

std::error_code LogFileImpl::Parse(
    const std::filesystem::path& file_path) noexcept {
  records_.clear();
  file_path_ = std::filesystem::path();
  std::error_code ec;
  auto file_size = std::filesystem::file_size(file_path, ec);
  if (ec) {
    return ec;
  }
  // mapped_file constructor will throw on empty files.
  if (file_size == 0) {
    file_path_ = file_path;
    return ErrorCodes::kOk;
  }
  mapped_file_ = boost::iostreams::mapped_file(
      file_path, boost::iostreams::mapped_file::readonly);
  if (!mapped_file_.is_open()) {
    return ErrorCodes::kFailedMapFile;
  }
  file_path_ = file_path;
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
