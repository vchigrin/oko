// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <vector>
#include <string>
#include "viewer/log_view.h"

namespace oko {

class LogFilter : public LogView {
 public:
  enum class Type {
    kIncludePattern,
    kExcludePattern,
    kLevel,
  };
  virtual size_t filtered_out_records_count() const noexcept = 0;
  virtual Type type() const noexcept = 0;
};

}  // namespace oko
