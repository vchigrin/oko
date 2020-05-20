// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/cache_directories_manager.h"

#include <boost/uuid/detail/sha1.hpp>
#include <boost/algorithm/hex.hpp>
#include <cstdlib>

namespace oko {

namespace {

std::string HashData(std::string_view data) noexcept {
  using boost::uuids::detail::sha1;
  sha1 hash;
  hash.process_bytes(data.data(), data.size());
  sha1::digest_type digest;
  hash.get_digest(digest);

  std::string result;
  boost::algorithm::hex(
      reinterpret_cast<const char*>(&digest),
      reinterpret_cast<const char*>(&digest) + sizeof(digest),
      std::back_inserter(result));
  return result;
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

std::filesystem::path CacheDirectoriesManager::DirectoryForS3Url(
    const std::string& s3_directory_url) noexcept {
  assert(!cache_root_path_.empty());
  std::filesystem::path result = cache_root_path_ / HashData(s3_directory_url);
  std::error_code ec;
  if (!std::filesystem::is_directory(result, ec) || ec) {
    std::filesystem::create_directories(result, ec);
    if (ec) {
      // TODO(vchigrin): Properly handle error;
      return {};
    }
  }
  return result;
}

}  // namespace oko
