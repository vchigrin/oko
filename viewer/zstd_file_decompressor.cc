// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/zstd_file_decompressor.h"

#include <zstd.h>

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <fstream>
#include <memory>
#include <vector>

#include "viewer/error_codes.h"

namespace oko {

std::optional<std::string> ZstdFileDecompressor::FileNameAfterDecompression(
    const std::string& file_name) const noexcept {
  const std::string_view kExtToStrip{".zst"};
  if (boost::algorithm::ends_with(file_name, kExtToStrip)) {
    return file_name.substr(0, file_name.length() - kExtToStrip.size());
  } else {
    return std::nullopt;
  }
}

std::error_code ZstdFileDecompressor::Decompress(
    const std::filesystem::path& src_file_path,
    const std::filesystem::path& dst_file_path) noexcept {
  std::ifstream src_file(src_file_path, std::ios::in | std::ios::binary);
  if (!src_file.is_open()) {
    return std::error_code(errno, std::generic_category());
  }
  std::ofstream dst_file(
      dst_file_path,
      std::ios::out | std::ios::binary | std::ios::trunc);
  if (!dst_file.is_open()) {
    return std::error_code(errno, std::generic_category());
  }
  std::unique_ptr<ZSTD_DStream, size_t(*)(ZSTD_DStream*)> decompressor(
      ZSTD_createDStream(),
      &ZSTD_freeDStream);
  if (!decompressor) {
    return ErrorCodes::kDecompressError;
  }
  size_t const init_result = ZSTD_initDStream(decompressor.get());
  std::vector<char> in_buffer(ZSTD_DStreamInSize());
  std::vector<char> out_buffer(ZSTD_DStreamOutSize());
  size_t next_input_block_size = init_result;
  while (next_input_block_size > 0) {
    in_buffer.resize(std::max(next_input_block_size, in_buffer.size()));
    src_file.read(in_buffer.data(), next_input_block_size);
    size_t bytes_read = src_file.gcount();
    if (bytes_read != next_input_block_size) {
      return ErrorCodes::kDecompressError;
    }
    ZSTD_inBuffer input = { in_buffer.data(), bytes_read, 0 };
    while (input.pos < input.size) {
      ZSTD_outBuffer output = { out_buffer.data(), out_buffer.size(), 0 };
      next_input_block_size = ZSTD_decompressStream(
          decompressor.get(), &output , &input);
      if (ZSTD_isError(next_input_block_size)) {
        return ErrorCodes::kDecompressError;
      }
      dst_file.write(out_buffer.data(), output.pos);
    }
  }
  return std::error_code();
}

}  // namespace oko
