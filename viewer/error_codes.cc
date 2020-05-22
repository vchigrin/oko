// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/error_codes.h"

#include <cassert>

namespace oko {

const char* CustomErrorCategory::name() const noexcept {
  return "OkoApplicationError";
}

// Return what each enum means in text
std::string CustomErrorCategory::message(int c) const {
  switch (static_cast<ErrorCodes>(c)) {
    case ErrorCodes::kOk:
      return "No error";
    case ErrorCodes::kFailedMapFile:
      return "Failed map file to memory";
    case ErrorCodes::kFileFormatCorrupted:
      return "File format is corrupted";
    case ErrorCodes::kFailedDownloadFile:
      return "Failed download file";
    default:
      assert(false);
      return "unknown";
  }
}

// static
const CustomErrorCategory& CustomErrorCategory::instance() {
  static CustomErrorCategory c;
  return c;
}

std::error_code make_error_code(ErrorCodes e) {
  return {static_cast<int>(e), CustomErrorCategory::instance()};
}

}  // namespace oko
