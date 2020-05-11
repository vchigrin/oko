// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <boost/program_options.hpp>

#include <ncurses.h>

#include <iostream>
#include <optional>

#include "viewer/add_pattern_filter_window.h"
#include "viewer/app_model.h"
#include "viewer/memorylog_log_file.h"
#include "viewer/ncurses_helpers.h"
#include "viewer/screen_layout.h"

namespace po = boost::program_options;

void ShowFile(std::unique_ptr<oko::MemorylogLogFile> file) {
  initscr();
  start_color();
  noecho();
  // Make all characters available immediately as typed.
  cbreak();
  // Invisible cursor.
  curs_set(0);
  keypad(stdscr, true);
  refresh();
  oko::AppModel model(std::move(file));
  oko::ScreenLayout screen_layout(&model);
  std::optional<oko::AddPatternFilterWindow> add_pattern_filter_window;

  bool should_run = true;
  while (should_run) {
    oko::StatusInfo info;
    info.file_name = model.file_path();
    info.total_records = screen_layout.log_window().total_records();
    info.marked_records = screen_layout.log_window().marked_records();
    info.marked_duration = screen_layout.log_window().marked_duration();
    screen_layout.status_window().UpdateStatus(info);
    screen_layout.Display();
    if (add_pattern_filter_window) {
      add_pattern_filter_window->Display();
    }
    int key = getch();
    if (add_pattern_filter_window) {
      add_pattern_filter_window->HandleKeyPress(key);
    } else {
      switch (key) {
        case 'q':
          should_run = false;
          break;
        case 'i':
          add_pattern_filter_window.emplace(/* is_include_filter */ true);
          break;
        case 'e':
          add_pattern_filter_window.emplace(/* is_include_filter */ false);
          break;
        case '=':
          model.RemoveAllFilters();
          break;
        case '-':
          model.RemoveLastFilter();
          break;
        default:
          screen_layout.HandleKeyPress(key);
      }
    }
    if (add_pattern_filter_window && add_pattern_filter_window->finished()) {
      std::string pattern = add_pattern_filter_window->entered_string();
      if (!pattern.empty()) {
        model.AppendFilter(
            std::move(pattern),
            add_pattern_filter_window->is_include_filter());
      }
      add_pattern_filter_window = std::nullopt;
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
