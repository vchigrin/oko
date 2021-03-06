// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <filesystem>
#include <system_error>
#include <vector>
#include "viewer/log_view.h"

namespace oko {

class LogFile : public LogView {
 public:
  virtual ~LogFile() = default;

  // Parses file, provided in constructor of concrete class.
  // May create inside memory view of the file, so it is expected
  // that file will not be changed or deleted during lifetime of this object.
  virtual std::error_code Parse() noexcept = 0;
  virtual const std::filesystem::path& file_path() const noexcept = 0;
};

}  // namespace oko
