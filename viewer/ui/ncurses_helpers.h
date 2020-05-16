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

// RAII wrapper around ncurses initialization and finalization code.
class WithTUI {
 public:
  WithTUI() noexcept {
    initscr();
    start_color();
    noecho();
    // Make all characters available immediately as typed.
    cbreak();
    // Invisible cursor.
    curs_set(0);
    keypad(stdscr, true);
    refresh();
  }

  ~WithTUI() {
    endwin();
  }
};

}  // namespace oko
