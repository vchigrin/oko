// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <system_error>
#include <string>
#include <type_traits>

namespace oko {

enum class ErrorCodes {
  kOk = 0,
  kFailedMapFile,
  kFileFormatCorrupted,
  kFailedDownloadFile,
};

// Define a custom error code category derived from std::error_category
class CustomErrorCategory : public std::error_category {
 public:
  const char* name() const noexcept override;
  std::string message(int c) const override;

  static const CustomErrorCategory& instance();
};

std::error_code make_error_code(ErrorCodes e);

}  // namespace oko

namespace std {
  template <> struct is_error_code_enum<oko::ErrorCodes> : true_type {
  };
}  // namespace std

