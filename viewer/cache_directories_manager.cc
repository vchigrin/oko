// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/cache_directories_manager.h"

#include <boost/algorithm/hex.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <cstdlib>
#include <utility>

#include "viewer/error_codes.h"

namespace oko {

namespace {

using Hasher = boost::uuids::detail::sha1;

std::string DigestToString(Hasher::digest_type digest) noexcept {
  std::string result;
  boost::algorithm::hex(
      reinterpret_cast<const char*>(digest),
      reinterpret_cast<const char*>(digest) + sizeof(Hasher::digest_type),
      std::back_inserter(result));
  return result;
}

std::string HashData(std::string_view data) noexcept {
  Hasher hash;
  hash.process_bytes(data.data(), data.size());
  Hasher::digest_type digest;
  hash.get_digest(digest);
  return DigestToString(digest);
}

outcome::std_result<std::string> HashFile(
    const std::filesystem::path& file_path) noexcept {
  Hasher hash;

  std::error_code ec;
  auto file_size = std::filesystem::file_size(file_path, ec);
  if (ec) {
    return ec;
  }
  if (file_size > 0) {
    // mapped_file constructor will throw on empty files.
    boost::iostreams::mapped_file mapped_file(file_path);
    if (!mapped_file.is_open()) {
      return ErrorCodes::kFailedMapFile;
    }
    hash.process_bytes(mapped_file.data(), mapped_file.size());
  }
  Hasher::digest_type digest;
  hash.get_digest(digest);
  return DigestToString(digest);
}

std::filesystem::path GetHomeDir() noexcept {
  const char* home = std::getenv("HOME");
  if (home) {
    return std::filesystem::path(home);
  } else {
    return {};
  }
}

}  // namespace

CacheDirectoriesManager::CacheDirectoriesManager() noexcept {
  std::filesystem::path home_dir = GetHomeDir();
  if (!home_dir.empty()) {
    cache_root_path_ = home_dir / ".cache/oko";
  }
}

outcome::std_result<std::filesystem::path>
    CacheDirectoriesManager::DirectoryForS3Url(
        const std::string& s3_directory_url) noexcept {
  return DirectoryForHash(HashData(s3_directory_url));
}

outcome::std_result<std::filesystem::path>
    CacheDirectoriesManager::DirectoryForHash(std::string hash) noexcept {
  assert(!cache_root_path_.empty());
  assert(!hash.empty());
  std::filesystem::path result = cache_root_path_ / hash;
  std::error_code ec;
  if (!std::filesystem::is_directory(result, ec) || ec) {
    std::filesystem::create_directories(result, ec);
    if (ec) {
      return ec;
    }
  }
  return result;
}

outcome::std_result<std::filesystem::path>
    CacheDirectoriesManager::DirectoryForFile(
        const std::filesystem::path& file_path) noexcept {
  auto maybe_hash = HashFile(file_path);
  if (!maybe_hash) {
    return maybe_hash.error();
  }
  return DirectoryForHash(std::move(maybe_hash.value()));
}

outcome::std_result<std::filesystem::path>
    CacheDirectoriesManager::DirectoryForData(std::string_view data) noexcept {
  return DirectoryForHash(HashData(data));
}

}  // namespace oko
