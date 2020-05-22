// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include<string>

#include "viewer/file_decompressor.h"

namespace oko {

class ZstdFileDecompressor : public FileDecompressor {
 public:
  std::optional<std::string> FileNameAfterDecompression(
      const std::string& file_name) const noexcept override;

  std::error_code Decompress(
      const std::filesystem::path& src_file_path,
      const std::filesystem::path& dst_file_path) noexcept override;
};

}  // namespace oko
