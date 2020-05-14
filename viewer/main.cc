// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <boost/program_options.hpp>

#include <ncurses.h>

#include <iostream>
#include <optional>

#include "viewer/add_pattern_filter_dialog.h"
#include "viewer/app_model.h"
#include "viewer/go_to_timestamp_dialog.h"
#include "viewer/memorylog_log_file.h"
#include "viewer/ncurses_helpers.h"
#include "viewer/screen_layout.h"
#include "viewer/search_dialog.h"
#include "viewer/text_log_file.h"

namespace po = boost::program_options;

void ShowFile(std::unique_ptr<oko::LogFile> file) {
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
  std::unique_ptr<oko::DialogWindow> current_dialog;

  bool should_run = true;
  while (should_run) {
    screen_layout.Display();
    if (current_dialog) {
      current_dialog->Display();
    }
    int key = getch();
    if (current_dialog) {
      current_dialog->HandleKeyPress(key);
    } else {
      switch (key) {
        case 'q':
          should_run = false;
          break;
        case 'i':
          current_dialog = std::make_unique<oko::AddPatternFilterDialog>(
              &model,
              /* is_include_filter */ true);
          break;
        case 'e':
          current_dialog = std::make_unique<oko::AddPatternFilterDialog>(
              &model,
              /* is_include_filter */ false);
          break;
        case '=':
          model.RemoveAllFilters();
          break;
        case '-':
          model.RemoveLastFilter();
          break;
        case 'g':
          current_dialog = std::make_unique<oko::GoToTimestampDialog>(&model);
          break;
        case '/':
          current_dialog = std::make_unique<oko::SearchDialog>(&model);
          break;
        case 'n':
          model.SearchNextEntry();
          break;
        case 'N':
          model.SearchPrevEntry();
          break;
        default:
          screen_layout.HandleKeyPress(key);
      }
    }
    if (current_dialog && current_dialog->finished()) {
      current_dialog.reset();
    }
  }
  endwin();
}

int main(int argc, char* argv[]) {
  po::variables_map vm;
  try {
    po::options_description desc;
    desc.add_options()
        ("help,h", "Help")
        ("memorylog,m", po::value<std::string>(), "Path to memorylog log file")
        ("textlog,t", po::value<std::string>(), "Path to text log file");
    po::store(
        po::command_line_parser(argc, argv).options(desc).run(),
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
  if (!(vm.count("memorylog") != 0 ^ vm.count("textlog") != 0)) {
    std::cerr << ("Eiter \"memorylog\" or \"textlog\" option "
        "must be present, but not both.") << std::endl;
    return 1;
  }


  std::unique_ptr<oko::LogFile> file;
  std::string log_path;
  if (vm.count("memorylog")) {
    file = std::make_unique<oko::MemorylogLogFile>();
    log_path = vm["memorylog"].as<std::string>();
  } else {
    file = std::make_unique<oko::TextLogFile>();
    log_path = vm["textlog"].as<std::string>();
  }
  if (!file->Parse(log_path)) {
    std::cerr << "Failed parse file " << log_path << std::endl;
    return 1;
  }
  ShowFile(std::move(file));
  return 0;
}
