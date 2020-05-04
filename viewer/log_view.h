// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <vector>
#include "viewer/log_record.h"

namespace oko {

// Abstract class that represents subset of the records in one or more
// log files.
class LogView {
 public:
  virtual ~LogView() = default;

  // Returned log records must be ordered by their timestamp.
  virtual const std::vector<LogRecord>& GetRecords() const noexcept = 0;
};

}  // namespace oko
