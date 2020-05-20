// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/zip_archive_files_provider.h"

#include <fstream>
#include <utility>

#include "viewer/log_formats/memorylog_log_file.h"
#include "viewer/log_formats/text_log_file.h"

namespace oko {

ZipArchiveFilesProvider::ZipArchiveFilesProvider(
    std::filesystem::path cache_directory_path,
    std::filesystem::path zip_file_path) noexcept
    : cache_directory_path_(std::move(cache_directory_path)),
      zip_file_(
          zip_open(zip_file_path.native().c_str(), ZIP_RDONLY, nullptr),
          &zip_close) {
}

std::vector<LogFileInfo> ZipArchiveFilesProvider::GetLogFileInfos() noexcept {
  if (!zip_file_) {
    // TODO(vchigrin): Better error reporting.
    return {};
  }
  auto num_entries = zip_get_num_entries(zip_file_.get(), 0);
  if (num_entries <= 0) {
    return {};
  }
  std::vector<LogFileInfo> result;
  result.reserve(num_entries);
  for (int64_t i = 0; i < num_entries; ++i) {
    struct zip_stat entry;
    if (zip_stat_index(zip_file_.get(), i, 0, &entry) != 0) {
      // TODO(vchigrin): Better error reporting.
      continue;
    }
    if (!(entry.valid & ZIP_STAT_NAME) || !(entry.valid & ZIP_STAT_SIZE)) {
      // TODO(vchigrin): Better error reporting.
      continue;
    }
    std::string file_name = entry.name;
    if (MemorylogLogFile::NameMatches(file_name) ||
        TextLogFile::NameMatches(file_name)) {
      name_to_index_[file_name] = i;
      result.emplace_back(LogFileInfo{std::move(file_name), entry.size});
    }
  }
  return result;
}

std::filesystem::path ZipArchiveFilesProvider::FetchLog(
    const std::string& log_file_name) noexcept {
  std::filesystem::path result_path = cache_directory_path_ / log_file_name;
  std::error_code ec;
  if (std::filesystem::exists(result_path, ec) && !ec) {
    return result_path;
  }

  auto it = name_to_index_.find(log_file_name);
  if (it == name_to_index_.end()) {
    return {};
  }
  std::unique_ptr<zip_file_t, int(*)(zip_file_t*)> zip_file(
      zip_fopen_index(zip_file_.get(), it->second, 0),
      &zip_fclose);
  if (!zip_file) {
    return {};
  }
  std::filesystem::create_directories(result_path.parent_path(), ec);
  if (ec) {
    return {};
  }

  std::filesystem::path tmp_file_path = result_path;
  tmp_file_path.concat(".tmp");
  {
    std::ofstream dst_file(
        tmp_file_path,
        std::ios::out | std::ios::binary | std::ios::trunc);
    if (!dst_file.is_open()) {
      return {};
    }
    std::vector<char> buf(4096);
    while (true) {
      auto result = zip_fread(zip_file.get(), buf.data(), buf.size());
      if (result <= 0) {
        break;
      }
      dst_file.write(buf.data(), result);
    }
  }
  std::filesystem::rename(tmp_file_path, result_path);
  return result_path;
}

}  // namespace oko
