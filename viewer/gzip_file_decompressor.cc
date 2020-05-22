// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/gzip_file_decompressor.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <fstream>

namespace oko {

namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;

std::optional<std::string> GzipFileDecompressor::FileNameAfterDecompression(
    const std::string& file_name) const noexcept {
  const std::string_view kExtToStrip{".gz"};
  if (boost::algorithm::ends_with(file_name, kExtToStrip)) {
    return file_name.substr(0, file_name.length() - kExtToStrip.size());
  } else {
    return std::nullopt;
  }
}

std::error_code GzipFileDecompressor::Decompress(
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

  boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
  out.push(boost::iostreams::gzip_decompressor());
  out.push(src_file);
  boost::iostreams::copy(out, dst_file);

  return std::error_code();
}

}  // namespace oko
