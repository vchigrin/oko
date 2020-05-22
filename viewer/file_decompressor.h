// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <boost/outcome/outcome.hpp>
#include <string>
#include <system_error>
#include <filesystem>

namespace oko {

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

// Base class for decompressors, that can decompress single log file.
class FileDecompressor {
 public:
  virtual ~FileDecompressor() = default;
  // Returns nullopt if this file can not be decompressed by this
  // decompressor.
  virtual std::optional<std::string> FileNameAfterDecompression(
      const std::string& file_name) const noexcept = 0;
  virtual std::error_code Decompress(
      const std::filesystem::path& src_file_path,
      const std::filesystem::path& dst_file_path) noexcept = 0;
};

}  // namespace oko
