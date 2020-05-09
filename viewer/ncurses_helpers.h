// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#pragma once
#include <ncurses.h>

#include <memory>

namespace oko {

class WithColor {
 public:
  WithColor(
      const std::unique_ptr<WINDOW, int(*)(WINDOW*)>& window,
      int pair)
      : window_(window),
        pair_(pair) {
    wattron(window_.get(), COLOR_PAIR(pair_));
  }

  ~WithColor() {
    wattroff(window_.get(), COLOR_PAIR(pair_));
  }

 private:
  const std::unique_ptr<WINDOW, int(*)(WINDOW*)>& window_;
  const int pair_;
};

// Log window
const int kTimeColorPair = 1;
const int kDebugColorPair = 2;
const int kInfoColorPair = 3;
const int kWarnColorPair = 4;
const int kErrColorPair = 5;
const int kMarkColorPair = 6;

// Status window
const int kStatusColorPair = 7;
const int kStatusMarkColorPair = 8;


inline void InitColors() {
  init_pair(kTimeColorPair, COLOR_YELLOW, COLOR_BLACK);
  init_pair(kDebugColorPair, COLOR_BLUE, COLOR_BLACK);
  init_pair(kInfoColorPair, COLOR_GREEN, COLOR_BLACK);
  init_pair(kWarnColorPair, COLOR_YELLOW, COLOR_BLACK);
  init_pair(kErrColorPair, COLOR_RED, COLOR_BLACK);
  init_pair(kMarkColorPair, COLOR_BLACK, COLOR_RED);

  init_pair(kStatusColorPair, COLOR_BLACK, COLOR_WHITE);
  init_pair(kStatusMarkColorPair, COLOR_RED, COLOR_WHITE);
}

}  // namespace oko
