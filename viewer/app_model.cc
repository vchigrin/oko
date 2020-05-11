// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/app_model.h"

#include <utility>

namespace oko {

AppModel::AppModel(std::unique_ptr<oko::LogFile> file)
    : file_(std::move(file)) {
}

AppModel::~AppModel() {
  for (auto* filter : active_filters_) {
    delete filter;
  }
}

void AppModel::AppendFilter(
    std::string pattern,
    bool is_include_filter) noexcept {
  oko::LogPatternFilter* new_filter = new oko::LogPatternFilter(
      active_view(),
      std::move(pattern),
      is_include_filter);
  active_filters_.push_back(new_filter);
  sig_filter_set_changed_(active_filters_);
}

}  // namespace oko
