// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/log_files_provider.h"

#include <utility>

#include "viewer/log_formats/memorylog_log_file.h"
#include "viewer/log_formats/text_log_file.h"
#include "viewer/gzip_file_decompressor.h"
#include "viewer/zstd_file_decompressor.h"

namespace oko {

namespace {

bool MatchesAnyLogFileFormat(const std::string& file_name) noexcept {
  return MemorylogLogFile::NameMatches(file_name) ||
      TextLogFile::NameMatches(file_name);
}


std::unique_ptr<LogFile> TryCreateFileForDecompressedPath(
    std::filesystem::path file_path) noexcept {
  if (oko::TextLogFile::NameMatches(file_path.filename())) {
    return std::make_unique<oko::TextLogFile>(std::move(file_path));
  } else if (oko::MemorylogLogFile::NameMatches(file_path.filename())) {
    return std::make_unique<oko::MemorylogLogFile>(std::move(file_path));
  }
  return nullptr;
}

}  // namespace

LogFilesProvider::LogFilesProvider(
      std::unique_ptr<CacheDirectoriesManager> cache_manager) noexcept
    : cache_manager_(std::move(cache_manager)) {
  decompressors_.emplace_back(std::make_unique<GzipFileDecompressor>());
  decompressors_.emplace_back(std::make_unique<ZstdFileDecompressor>());
}

bool LogFilesProvider::CanBeLogFileName(
    const std::string& file_name) const noexcept {
  if (MatchesAnyLogFileFormat(file_name)) {
    return true;
  }
  for (const auto& decompressor : decompressors_) {
    auto maybe_decompressed = decompressor->FileNameAfterDecompression(
        file_name);
    if (!maybe_decompressed) {
      continue;
    }
    if (MatchesAnyLogFileFormat(*maybe_decompressed)) {
      return true;
    }
  }
  return false;
}

outcome::std_result<std::unique_ptr<LogFile>>
    LogFilesProvider::CreateFileForPath(
        std::filesystem::path file_path) const noexcept {
  if (auto maybe_result = TryCreateFileForDecompressedPath(file_path)) {
    return maybe_result;
  }
  for (const auto& decompressor : decompressors_) {
    auto maybe_file_name = decompressor->FileNameAfterDecompression(
        file_path.filename());
    if (!maybe_file_name) {
      continue;
    }
    auto maybe_cache_dir = cache_manager_->DirectoryForFile(
        file_path);
    if (!maybe_cache_dir) {
      return maybe_cache_dir.error();
    }
    std::error_code ec;
    std::filesystem::path dst_path =
        maybe_cache_dir.value() / *maybe_file_name;
    if (!std::filesystem::exists(dst_path, ec) || ec) {
      ec = decompressor->Decompress(file_path, dst_path);
      if (ec) {
        return ec;
      }
    }
    std::unique_ptr<LogFile> result = TryCreateFileForDecompressedPath(
        std::move(dst_path));
    if (result) {
      return result;
    }
    assert(false);
    std::abort();
  }

  assert(false);
  std::abort();
}

}  // namespace oko
