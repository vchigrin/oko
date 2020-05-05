// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <boost/program_options.hpp>
#include <iostream>
#include <ncurses.h>

#include "viewer/memorylog_log_file.h"
#include "viewer/log_window.h"

namespace po = boost::program_options;

void ShowFile(std::unique_ptr<oko::MemorylogLogFile> file) {
  initscr();
  start_color();
  noecho();
  // Invisible cursor.
  curs_set(0);
  keypad(stdscr, true);
  int max_row = 0, max_col = 0;
  getmaxyx(stdscr, max_row, max_col);
  refresh();
  oko::LogWindow log_window(std::move(file), 0, 0, max_row, max_col);

  bool should_run = true;
  while (should_run) {
    log_window.Display();
    int key = getch();
    switch (key) {
      case 'q':
        should_run = false;
        break;
      default:
        log_window.HandleKeyPress(key);
    }
  }
  endwin();
}

void AnalyzeFile(const std::string& log_path) noexcept {
  std::unique_ptr<oko::MemorylogLogFile> file(
      std::make_unique<oko::MemorylogLogFile>());
  if (!file->Parse(log_path)) {
    std::cerr << "Failed parse file " << log_path << std::endl;
    return;
  }
  ShowFile(std::move(file));
}

int main(int argc, char* argv[]) {
  po::variables_map vm;
  try {
    po::options_description desc;
    desc.add_options()
        ("help,h", "Help")
        ("log_path", po::value<std::string>()->required(), "Path to log file");
    po::positional_options_description p;
    p.add("log_path", 1);

    po::store(
        po::command_line_parser(argc, argv).options(desc).positional(p).run(),
        vm);
    if (vm.count("help")) {
      std::cout << desc << std::endl;
    }
    po::notify(vm);
  }
  catch (const po::error &ex) {
    std::cerr << ex.what() << std::endl;
    return 1;
  }
  AnalyzeFile(vm["log_path"].as<std::string>());
  return 0;
}
