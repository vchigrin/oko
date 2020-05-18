// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/merged_log_view.h"

#include <algorithm>
#include <functional>
#include <numeric>
#include <queue>
#include <utility>

namespace oko {

namespace {

struct RecordListWithIt {
  const std::vector<LogRecord>* container;
  std::vector<LogRecord>::const_iterator next_pos;

  explicit RecordListWithIt(const std::vector<LogRecord>* c) noexcept
      : container(c),
        next_pos(c->begin()) {
  }

  bool operator < (const RecordListWithIt& second) const noexcept {
    return next_pos->timestamp() < second.next_pos->timestamp();
  }

  bool operator > (const RecordListWithIt& second) const noexcept {
    return next_pos->timestamp() > second.next_pos->timestamp();
  }
};

}  // namespace

MergedLogView::MergedLogView(const std::vector<LogView*>& views) noexcept {
  std::vector<RecordListWithIt> merged_lists;
  merged_lists.reserve(views.size());
  for (const LogView* v : views) {
    const std::vector<LogRecord>& records = v->GetRecords();
    if (records.empty()) {
      continue;
    }
    merged_lists.emplace_back(RecordListWithIt(&records));
  }
  const size_t result_size = std::accumulate(
      merged_lists.begin(),
      merged_lists.end(),
      static_cast<size_t>(0),
      [](size_t l, const RecordListWithIt& r) {
        return l + r.container->size();
      });
  records_.reserve(result_size);
  // top - is the record list with smallest timestamp of the next record.
  std::priority_queue<
      RecordListWithIt,
      std::vector<RecordListWithIt>,
      std::greater<RecordListWithIt>> merged_queue(
          std::greater<RecordListWithIt>(),
          std::move(merged_lists));
  while (!merged_queue.empty()) {
    RecordListWithIt top = merged_queue.top();
    merged_queue.pop();
    records_.emplace_back(*top.next_pos);
    ++top.next_pos;
    if (top.next_pos != top.container->end()) {
      merged_queue.emplace(std::move(top));
    }
  }
}

const std::vector<LogRecord>& MergedLogView::GetRecords() const noexcept {
  return records_;
}

}  // namespace oko
