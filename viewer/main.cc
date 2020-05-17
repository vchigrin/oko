// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.
#include <boost/program_options.hpp>

#include <ncurses.h>

#include <chrono>
#include <filesystem>
#include <future>
#include <iostream>
#include <optional>

#include "viewer/app_model.h"
#include "viewer/directory_log_files_provider.h"
#include "viewer/log_formats/memorylog_log_file.h"
#include "viewer/log_formats/text_log_file.h"
#include "viewer/ui/add_pattern_filter_dialog.h"
#include "viewer/ui/go_to_timestamp_dialog.h"
#include "viewer/ui/log_files_window.h"
#include "viewer/ui/ncurses_helpers.h"
#include "viewer/ui/progress_window.h"
#include "viewer/ui/screen_layout.h"
#include "viewer/ui/search_dialog.h"
#include "viewer/ui/search_log_dialog.h"

namespace po = boost::program_options;

void ShowFile(std::unique_ptr<oko::LogFile> file) {
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
}

std::filesystem::path RunChooseFileInDirectory(
    const std::string& directory_path) noexcept {
  oko::DirectoryLogFilesProvider provider(directory_path);
  int num_rows = 0, num_columns = 0;
  getmaxyx(stdscr, num_rows, num_columns);
  oko::LogFilesWindow window(&provider, 0, 0, num_rows, num_columns);
  if (!window.has_any_file_infos()) {
    return std::filesystem::path();
  }
  std::unique_ptr<oko::DialogWindow> current_dialog;

  while (true) {
    window.Display();
    if (current_dialog) {
      current_dialog->Display();
    }
    int key = getch();
    if (current_dialog) {
      current_dialog->HandleKeyPress(key);
    } else {
      switch (key) {
        case 'q':
          return std::filesystem::path();
        case '/':
          current_dialog = std::make_unique<oko::SearchLogDialog>(&window);
          break;
        case 'n':
          window.SearchNextEntry();
          break;
        case 'N':
          window.SearchPrevEntry();
          break;
        default:
          window.HandleKeyPress(key);
      }
    }
    if (current_dialog && current_dialog->finished()) {
      current_dialog.reset();
    }
    if (window.finished()) {
      return window.fetched_file_path();
    }
  }
}

int main(int argc, char* argv[]) {
  po::variables_map vm;
  try {
    po::options_description desc;
    desc.add_options()
        ("help,h", "Help")
        ("memorylog,m", po::value<std::string>(), "Path to memorylog log file")
        ("textlog,t", po::value<std::string>(), "Path to text log file")
        ("directory,d", po::value<std::string>(),
            "Path to directory with log files");
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
  const int options_count =
      vm.count("memorylog") + vm.count("textlog") + vm.count("directory");
  if (options_count != 1) {
    std::cerr << ("Eiter \"memorylog\" or \"textlog\" or \"directory\" option "
        "must be present, end exactly one.") << std::endl;
    return 1;
  }

  std::unique_ptr<oko::LogFile> file;
  std::filesystem::path log_path;
  oko::WithTUI tui_initializer;
  if (vm.count("memorylog")) {
    file = std::make_unique<oko::MemorylogLogFile>();
    log_path = vm["memorylog"].as<std::string>();
  } else if (vm.count("textlog")) {
    file = std::make_unique<oko::TextLogFile>();
    log_path = vm["textlog"].as<std::string>();
  } else if (vm.count("directory")) {
    log_path = RunChooseFileInDirectory(vm["directory"].as<std::string>());
    if (log_path.empty()) {
      return 1;
    }
    if (oko::TextLogFile::NameMatches(log_path.filename())) {
      file = std::make_unique<oko::TextLogFile>();
    } else if (oko::MemorylogLogFile::NameMatches(log_path.filename())) {
      file = std::make_unique<oko::MemorylogLogFile>();
    } else {
      assert(false);
    }
  } else {
    // Check in code above should exit program in this case.
    assert(false);
    return 1;
  }
  std::future<bool> parse_async = std::async(
      std::launch::async,
      [&file, &log_path] {
          return file->Parse(log_path);
      });
  {
    oko::ProgressWindow parse_file_window(
        "Parsing file...",
        [&parse_async] {
            return parse_async.wait_for(
                std::chrono::seconds(0)) == std::future_status::ready;
        });
    parse_file_window.PostSync();
  }
  if (bool parse_success = parse_async.get(); !parse_success) {
    std::cerr << "Failed parse file " << log_path << std::endl;
    return 1;
  }
  ShowFile(std::move(file));
  return 0;
}
