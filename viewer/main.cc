// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.
#include <ncurses.h>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <filesystem>
#include <future>
#include <iostream>
#include <optional>

#include "viewer/app_model.h"
#include "viewer/cache_directories_manager.h"
#include "viewer/directory_log_files_provider.h"
#include "viewer/log_formats/memorylog_log_file.h"
#include "viewer/log_formats/text_log_file.h"
#include "viewer/s3_log_files_provider.h"
#include "viewer/ui/add_level_filter_dialog.h"
#include "viewer/ui/add_pattern_filter_dialog.h"
#include "viewer/ui/go_to_timestamp_dialog.h"
#include "viewer/ui/log_files_window.h"
#include "viewer/ui/message_window.h"
#include "viewer/ui/ncurses_helpers.h"
#include "viewer/ui/progress_window.h"
#include "viewer/ui/screen_layout.h"
#include "viewer/ui/search_dialog.h"
#include "viewer/ui/search_log_dialog.h"
#include "viewer/zip_archive_files_provider.h"

namespace po = boost::program_options;

static const char kMainHelpMessage[] = (
  "F1              Show this help message\n"
  "F2, =           Remove all filters\n"
  "F3, -           Remove last added filter\n"
  "F4, g           Go to timestamp\n"
  "F5, i           Add include pattern filter\n"
  "F6, e           Add exlude pattern filter\n"
  "F7, /           Search for pattern\n"
  "F8, n           Search next pattern occurence\n"
  "N               Search prev pattern occurence\n"
  "F9, v           Add log level filter\n"
  "F10, m          Toggle marking mode\n"
  "F11, q          Exit\n"
  "F12, t          Toggle time format\n"
  "j, down arrow   One line down\n"
  "k, up arrow     One line up\n"
  "h, left arrow   Scroll left\n"
  "l, right arrow  Scroll right\n"
);

static const char kFileChooserHelpMessage[] = (
  "F1              Show this help message\n"
  "F7, /           Search for pattern\n"
  "F8, n           Search next pattern occurence\n"
  "F9, N           Search prev pattern occurence\n"
  "F10, m          Mark current file\n"
  "F11, q          Exit\n"
  "j, down arrow   One line down\n"
  "k, up arrow     One line up\n"
);

void ConfigureFunctionLabels(oko::FunctionBarWindow& wnd) noexcept {
  wnd.SetLabel(1, "Help");
  wnd.SetLabel(2, "RmAllFilters");
  wnd.SetLabel(3, "RmLastFilter");
  wnd.SetLabel(4, "GoToTimestam");
  wnd.SetLabel(5, "InclFilter");
  wnd.SetLabel(6, "ExlFilter");
  wnd.SetLabel(7, "Search");
  wnd.SetLabel(8, "SearchNext");
  wnd.SetLabel(9, "LevelFilter");
  wnd.SetLabel(10, "ToggleMark");
  wnd.SetLabel(11, "Quit");
  wnd.SetLabel(12, "Toggle time format");
}

void ShowFiles(std::vector<std::unique_ptr<oko::LogFile>> files) {
  oko::AppModel model(std::move(files));
  oko::ScreenLayout screen_layout(&model);
  ConfigureFunctionLabels(screen_layout.function_bar_window());
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
        case KEY_F(1):
          oko::MessageWindow::PostSync(kMainHelpMessage);
          break;
        case 'q':
        case KEY_F(11):
          should_run = false;
          break;
        case 'i':
        case KEY_F(5):
          current_dialog = std::make_unique<oko::AddPatternFilterDialog>(
              &model,
              /* is_include_filter */ true);
          break;
        case 'e':
        case KEY_F(6):
          current_dialog = std::make_unique<oko::AddPatternFilterDialog>(
              &model,
              /* is_include_filter */ false);
          break;
        case 'v':
        case KEY_F(9):
          current_dialog = std::make_unique<oko::AddLevelFilterDialog>(
              &model);
          break;
        case '=':
        case KEY_F(2):
          model.RemoveAllFilters();
          break;
        case '-':
        case KEY_F(3):
          model.RemoveLastFilter();
          break;
        case 'g':
        case KEY_F(4):
          current_dialog = std::make_unique<oko::GoToTimestampDialog>(&model);
          break;
        case '/':
        case KEY_F(7):
          current_dialog = std::make_unique<oko::SearchDialog>(&model);
          break;
        case 'n':
        case KEY_F(8):
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

std::vector<std::unique_ptr<oko::LogFile>> RunChooseFile(
    oko::LogFilesProvider& files_provider) noexcept {
  int num_rows = 0, num_columns = 0;
  getmaxyx(stdscr, num_rows, num_columns);
  oko::LogFilesWindow window(
      &files_provider,
      0, 0, num_rows - oko::FunctionBarWindow::kRows, num_columns);
  oko::FunctionBarWindow func_window(
      num_rows - oko::FunctionBarWindow::kRows, 0, num_columns);
  func_window.SetLabel(1, "Help");
  func_window.SetLabel(7, "Search");
  func_window.SetLabel(8, "SearchNext");
  func_window.SetLabel(9, "SearchPrev");
  func_window.SetLabel(10, "ToggleMark");
  func_window.SetLabel(11, "Quit");
  std::unique_ptr<oko::DialogWindow> current_dialog;

  while (!window.finished()) {
    window.Display();
    func_window.Display();
    if (current_dialog) {
      current_dialog->Display();
    }
    int key = getch();
    if (current_dialog) {
      current_dialog->HandleKeyPress(key);
    } else {
      switch (key) {
        case KEY_F(1):
          oko::MessageWindow::PostSync(kFileChooserHelpMessage);
          break;
        case 'q':
        case KEY_F(11):
          return {};
        case '/':
        case KEY_F(7):
          current_dialog = std::make_unique<oko::SearchLogDialog>(&window);
          break;
        case 'n':
        case KEY_F(8):
          window.SearchNextEntry();
          break;
        case 'N':
        case KEY_F(9):
          window.SearchPrevEntry();
          break;
        default:
          window.HandleKeyPress(key);
      }
    }
    if (current_dialog && current_dialog->finished()) {
      current_dialog.reset();
    }
  }
  return window.RetrieveFetchedFiles();
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
            "Path to directory with log files")
        ("s3", po::value<std::string>(), "S3 folder URL")
        ("zip,z", po::value<std::string>(), "Path to .zip file with logs");
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

  if (vm.size() != 1) {
    std::cerr << "Exactly one program option must be passed." << std::endl;
    return 1;
  }

  std::vector<std::unique_ptr<oko::LogFile>> files;
  oko::WithTUI tui_initializer;
  std::unique_ptr<oko::CacheDirectoriesManager> cache_manager =
      std::make_unique<oko::CacheDirectoriesManager>();
  if (!cache_manager->is_initialized()) {
    oko::MessageWindow::PostSync("Failed initialize cache");
    return 1;
  }
  if (vm.count("memorylog")) {
    files.emplace_back(std::make_unique<oko::MemorylogLogFile>(
        vm["memorylog"].as<std::string>()));
  } else if (vm.count("textlog")) {
    files.emplace_back(std::make_unique<oko::TextLogFile>(
        vm["textlog"].as<std::string>()));
  } else if (vm.count("directory") || vm.count("s3") || vm.count("zip")) {
    std::unique_ptr<oko::LogFilesProvider> provider;
    if (vm.count("directory")) {
      provider = std::make_unique<oko::DirectoryLogFilesProvider>(
          std::move(cache_manager),
          vm["directory"].as<std::string>());
    } else if (vm.count("zip")) {
      std::string file_path = vm["zip"].as<std::string>();
      provider = std::make_unique<oko::ZipArchiveFilesProvider>(
          std::move(cache_manager),
          std::move(file_path));
    } else if (vm.count("s3")) {
      std::string s3_url = vm["s3"].as<std::string>();
      oko::outcome::std_result<std::filesystem::path> maybe_cache_dir =
          cache_manager->DirectoryForS3Url(s3_url);
      if (!maybe_cache_dir) {
        oko::MessageWindow::PostSync(boost::str(boost::format(
            "Failed initialize cache entry. %1%.") %
                maybe_cache_dir.error().message()));
        return 1;
      }
      provider = std::make_unique<oko::S3LogFilesProvider>(
          std::move(cache_manager),
          std::move(maybe_cache_dir.value()),
          std::move(s3_url));
    }
    files = RunChooseFile(*provider);
    if (files.empty()) {
      return 1;
    }
  } else {
    // Check in code above should exit program in this case.
    assert(false);
    return 1;
  }
  std::future<std::error_code> parse_async = std::async(
      std::launch::async,
      [&files] {
        for (const auto& file : files) {
          std::error_code ec = file->Parse();
          if (ec) {
            return ec;
          }
        }
        return std::error_code();
      });
  {
    oko::ProgressWindow parse_file_window(
        "Parsing files...",
        [&parse_async] {
            return parse_async.wait_for(
                std::chrono::seconds(0)) == std::future_status::ready;
        });
    parse_file_window.PostSync();
  }
  if (std::error_code parse_result = parse_async.get(); parse_result) {
    oko::MessageWindow::PostSync(boost::str(boost::format(
        "Failed parse file. %1%.") % parse_result.message()));
    return 1;
  }
  ShowFiles(std::move(files));
  return 0;
}
