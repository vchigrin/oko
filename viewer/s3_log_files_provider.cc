// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/s3_log_files_provider.h"

// Macro provided by curses.h.
#pragma push_macro("OK")
#undef OK
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#pragma pop_macro("OK")
#include <boost/iostreams/copy.hpp>
#include <fstream>
#include <regex>
#include <utility>

#include "viewer/error_codes.h"

namespace oko {

namespace {

static const char kUrl[] = "^https?://([^\\.]+)\\.[^/]*/(.*)";

bool SplitUrl(
    std::string url,
    std::string* bucket_name,
    std::string* s3_directory_name) {
  std::regex re_url(kUrl);
  std::smatch match_results;
  if (!std::regex_match(url, match_results, re_url)) {
    assert(false);
    return false;
  }
  *bucket_name = match_results[1];
  *s3_directory_name = match_results[2];
  return true;
}

}  // namespace

// Lists log files in directory, non-recursively.
S3LogFilesProvider::S3LogFilesProvider(
    std::filesystem::path cache_directory_path,
    std::string s3_directory_url) noexcept
    : cache_directory_path_(std::move(cache_directory_path)) {
  SplitUrl(std::move(s3_directory_url), &bucket_name_, &s3_directory_name_);
  if (!s3_directory_name_.empty() && s3_directory_name_.back() != '/') {
    s3_directory_name_ += '/';
  }
}

S3LogFilesProvider::~S3LogFilesProvider() {
  if (s3_client_) {
    s3_client_.reset();
    Aws::ShutdownAPI(aws_options_);
  }
}

void S3LogFilesProvider::EnsureInitialized() noexcept {
  if (!s3_client_) {
    Aws::InitAPI(aws_options_);
    s3_client_ = std::make_unique<Aws::S3::S3Client>();
  }
}

outcome::std_result<std::vector<LogFileInfo>>
    S3LogFilesProvider::GetLogFileInfos() noexcept {
  if (bucket_name_.empty()) {
    return ErrorCodes::kFailedDownloadFile;
  }

  Aws::S3::Model::ListObjectsRequest objects_request;
  objects_request.SetBucket(bucket_name_.c_str());
  objects_request.SetPrefix(s3_directory_name_.c_str());

  EnsureInitialized();
  auto list_objects_result = s3_client_->ListObjects(objects_request);

  if (!list_objects_result.IsSuccess()) {
    // TODO(vchigrin): provide detailed error description.
    return ErrorCodes::kFailedDownloadFile;
  }
  Aws::Vector<Aws::S3::Model::Object> objects =
      list_objects_result.GetResult().GetContents();

  std::vector<LogFileInfo> result;
  result.reserve(objects.size());
  const size_t directoryn_name_len = s3_directory_name_.length();
  for (const Aws::S3::Model::Object& s3_obj : objects) {
    const auto& key = s3_obj.GetKey();
    if (key.size() <= directoryn_name_len) {
      assert(false);  // S3 API does not respect Prefix in our request?
      continue;
    }
    std::string file_name(
        key.data() + directoryn_name_len,
        key.size() - directoryn_name_len);
    if (file_name.find('/') != std::string::npos) {
      // List files non-recursively.
      continue;
    }
    if (CanBeLogFileName(file_name)) {
      result.emplace_back(LogFileInfo{
          std::move(file_name),
          static_cast<uint64_t>(s3_obj.GetSize())
      });
    }
  }
  return result;
}

outcome::std_result<std::unique_ptr<LogFile>>
S3LogFilesProvider::FetchLog(const std::string& log_file_name) noexcept {
  std::filesystem::path dst_path = cache_directory_path_ / log_file_name;
  std::error_code ec;
  if (std::filesystem::exists(dst_path, ec) && !ec) {
    return CreateFileForPath(dst_path);
  }
  Aws::S3::Model::GetObjectRequest object_request;
  object_request.SetBucket(bucket_name_.c_str());
  std::string key = s3_directory_name_ + log_file_name;
  object_request.SetKey(key.c_str());
  EnsureInitialized();
  auto outcome = s3_client_->GetObject(object_request);
  if (!outcome.IsSuccess()) {
    // TODO(vchigrin): provide detailed error description.
    return ErrorCodes::kFailedDownloadFile;
  }
  Aws::S3::Model::GetObjectResult result = outcome.GetResultWithOwnership();
  std::iostream& retrieved_file = result.GetBody();
  std::filesystem::path tmp_file_path = dst_path;
  tmp_file_path.concat(".tmp");
  {
    std::ofstream dst_file(
        tmp_file_path,
        std::ios::out | std::ios::binary | std::ios::trunc);
    if (!dst_file.is_open()) {
      return std::error_code(errno, std::generic_category());
    }
    size_t copied_size = boost::iostreams::copy(
         retrieved_file,
         dst_file);
    if (copied_size != result.GetContentLength()) {
      // Not all data copied.
      return ErrorCodes::kFailedDownloadFile;
    }
  }
  std::filesystem::rename(tmp_file_path, dst_path);
  return CreateFileForPath(dst_path);
}

}  // namespace oko
