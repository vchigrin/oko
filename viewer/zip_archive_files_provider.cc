// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/zip_archive_files_provider.h"

#include <cstdlib>
#include <fstream>
#include <utility>

#include "viewer/error_codes.h"

namespace oko {

ZipArchiveFilesProvider::ZipArchiveFilesProvider(
    std::unique_ptr<CacheDirectoriesManager> cache_manager,
    std::filesystem::path zip_file_path) noexcept
    : LogFilesProvider(std::move(cache_manager)),
      zip_file_(
          zip_open(zip_file_path.native().c_str(), ZIP_RDONLY, nullptr),
          &zip_close) {
}

outcome::std_result<std::vector<LogFileInfo>>
    ZipArchiveFilesProvider::GetLogFileInfos() noexcept {
  if (!zip_file_) {
    return ErrorCodes::kFileFormatCorrupted;
  }
  auto num_entries = zip_get_num_entries(zip_file_.get(), 0);
  if (num_entries <= 0) {
    return ErrorCodes::kFileFormatCorrupted;
  }
  std::vector<LogFileInfo> result;
  result.reserve(num_entries);
  for (int64_t i = 0; i < num_entries; ++i) {
    struct zip_stat entry;
    zip_stat_init(&entry);
    if (zip_stat_index(zip_file_.get(), i, 0, &entry) != 0) {
      return ErrorCodes::kFileFormatCorrupted;
    }
    if (!(entry.valid & ZIP_STAT_NAME) || !(entry.valid & ZIP_STAT_SIZE)) {
      return ErrorCodes::kFileFormatCorrupted;
    }
    std::string file_name = entry.name;
    std::filesystem::path file_path(file_name);
    if (CanBeLogFileName(file_path.filename().native())) {
      name_to_index_[file_name] = i;
      result.emplace_back(LogFileInfo{std::move(file_name), entry.size});
    }
  }
  return result;
}

outcome::std_result<std::vector<char>>
    ZipArchiveFilesProvider::ReadCompressedData(
        zip_uint64_t entry_index) noexcept {
  std::unique_ptr<zip_file_t, int(*)(zip_file_t*)> zip_file(
      zip_fopen_index(zip_file_.get(), entry_index, ZIP_FL_COMPRESSED),
      &zip_fclose);
  if (!zip_file) {
    return ErrorCodes::kFileFormatCorrupted;
  }
  struct zip_stat entry;
  zip_stat_init(&entry);
  if (zip_stat_index(zip_file_.get(), entry_index, 0, &entry) != 0) {
    return ErrorCodes::kFileFormatCorrupted;
  }
  if (!(entry.valid & ZIP_STAT_COMP_SIZE)) {
    return ErrorCodes::kFileFormatCorrupted;
  }
  std::vector<char> buf(entry.comp_size);
  auto bytes_read = zip_fread(zip_file.get(), buf.data(), buf.size());
  if (bytes_read != entry.comp_size) {
    return ErrorCodes::kFileFormatCorrupted;
  }
  return buf;
}

outcome::std_result<std::unique_ptr<LogFile>>
    ZipArchiveFilesProvider::FetchLog(
        const std::string& log_file_name) noexcept {
  auto it = name_to_index_.find(log_file_name);
  if (it == name_to_index_.end()) {
    assert(false);
    std::abort();
  }
  outcome::std_result<std::vector<char>> compressed_data_buf =
      ReadCompressedData(it->second);
  if (!compressed_data_buf) {
    return compressed_data_buf.error();
  }
  outcome::std_result<std::filesystem::path> maybe_cache_directory_path =
      cache_manager_->DirectoryForData(
          std::string_view{
            compressed_data_buf.value().data(),
            compressed_data_buf.value().size()
      });
  if (!maybe_cache_directory_path) {
    return maybe_cache_directory_path.error();
  }

  std::filesystem::path result_path =
      maybe_cache_directory_path.value() / log_file_name;
  std::error_code ec;
  if (std::filesystem::exists(result_path, ec) && !ec) {
    return CreateFileForPath(result_path);
  }

  std::unique_ptr<zip_file_t, int(*)(zip_file_t*)> zip_file(
      zip_fopen_index(zip_file_.get(), it->second, 0),
      &zip_fclose);
  if (!zip_file) {
    return ErrorCodes::kFileFormatCorrupted;
  }
  std::filesystem::create_directories(result_path.parent_path(), ec);
  if (ec) {
    return ec;
  }

  std::filesystem::path tmp_file_path = result_path;
  tmp_file_path.concat(".tmp");
  {
    std::ofstream dst_file(
        tmp_file_path,
        std::ios::out | std::ios::binary | std::ios::trunc);
    if (!dst_file.is_open()) {
      return std::error_code(errno, std::generic_category());
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
  return CreateFileForPath(result_path);
}

}  // namespace oko
