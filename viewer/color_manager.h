// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <ncurses.h>

#include <unordered_map>
#include <utility>

namespace oko {

class ColorManager {
 public:
  static ColorManager& instance() noexcept;

  int RegisterColorPair(int foreground, int backgroud) noexcept;

  ColorManager(const ColorManager&) = delete;
  const ColorManager& operator = (const ColorManager&) = delete;

 private:
  ColorManager() = default;

  using Key = std::pair<int, int>;

  struct KeyHash {
    size_t operator()(const Key& k) const noexcept {
      const std::hash<int> hasher;
      return hasher(k.first) ^ hasher(k.second);
    }
  };

  std::unordered_map<Key, int, KeyHash> registered_colors_;
};

}  // namespace oko
