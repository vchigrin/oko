// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "viewer/ui/color_manager.h"

namespace oko {

// static
ColorManager& ColorManager::instance() noexcept {
  static ColorManager instance;
  return instance;
}

int ColorManager::RegisterColorPair(int foreground, int backgroud) noexcept {
  Key key{foreground, backgroud};
  auto it = registered_colors_.find(key);
  if (it != registered_colors_.end()) {
    return it->second;
  }
  int result = registered_colors_.size() + 1;
  init_pair(result, foreground, backgroud);
  registered_colors_[key] = result;
  return result;
}

}  // namespace oko
