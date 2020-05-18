// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <vector>
#include "viewer/log_view.h"

namespace oko {

// Merges records from several views, in timestamp order.
class MergedLogView : public LogView {
 public:
  MergedLogView(const std::vector<LogView*>& views) noexcept;
  // Returned log records must be ordered by their timestamp.
  const std::vector<LogRecord>& GetRecords() const noexcept override;

 private:
  std::vector<LogRecord> records_;
};

}  // namespace oko
