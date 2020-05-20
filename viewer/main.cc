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
#include "viewer/cache_directories_manager.h"
#include "viewer/directory_log_files_provider.h"
#include "viewer/log_formats/memorylog_log_file.h"
#include "viewer/log_formats/text_log_file.h"
#include "viewer/s3_log_files_provider.h"
#include "viewer/ui/add_pattern_filter_dialog.h"
#include "viewer/ui/go_to_timestamp_dialog.h"
#include "viewer/ui/log_files_window.h"
#include "viewer/ui/ncurses_helpers.h"
#include "viewer/ui/progress_window.h"
#include "viewer/ui/screen_layout.h"
#include "viewer/ui/search_dialog.h"
#include "viewer/ui/search_log_dialog.h"
#include "viewer/zip_archive_files_provider.h"

namespace po = boost::program_options;

void ConfigureFunctionLabels(oko::FunctionBarWindow& wnd) noexcept {
  wnd.SetLabel(2, "RmAllFilters");
  wnd.SetLabel(3, "RmLastFilter");
  wnd.SetLabel(4, "GoToTimestam");
  wnd.SetLabel(5, "InclFilter");
  wnd.SetLabel(6, "ExlFilter");
  wnd.SetLabel(7, "Search");
  wnd.SetLabel(8, "SearchNext");
  wnd.SetLabel(9, "SearchPrev");
  wnd.SetLabel(10, "ToggleMark");
  wnd.SetLabel(11, "Quit");
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
        case KEY_F(9):
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

std::vector<std::filesystem::path> RunChooseFile(
    oko::LogFilesProvider& files_provider) noexcept {
  int num_rows = 0, num_columns = 0;
  getmaxyx(stdscr, num_rows, num_columns);
  oko::LogFilesWindow window(
      &files_provider,
      0, 0, num_rows - oko::FunctionBarWindow::kRows, num_columns);
  oko::FunctionBarWindow func_window(
      num_rows - oko::FunctionBarWindow::kRows, 0, num_columns);
  func_window.SetLabel(11, "Quit");
  func_window.SetLabel(7, "Search");
  func_window.SetLabel(8, "SearchNext");
  func_window.SetLabel(9, "SearchPrev");
  func_window.SetLabel(10, "ToggleMark");
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
  return window.fetched_file_paths();
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

  struct FileWithPath {
    std::unique_ptr<oko::LogFile> file;
    std::filesystem::path log_path;
  };
  std::vector<FileWithPath> files_with_paths;
  oko::WithTUI tui_initializer;
  if (vm.count("memorylog")) {
    files_with_paths.push_back({
        std::make_unique<oko::MemorylogLogFile>(),
        vm["memorylog"].as<std::string>()});
  } else if (vm.count("textlog")) {
    files_with_paths.push_back({
        std::make_unique<oko::TextLogFile>(),
        vm["textlog"].as<std::string>()});
  } else if (vm.count("directory") || vm.count("s3") || vm.count("zip")) {
    std::unique_ptr<oko::LogFilesProvider> provider;
    if (vm.count("directory")) {
      provider = std::make_unique<oko::DirectoryLogFilesProvider>(
          vm["directory"].as<std::string>());
    } else if (vm.count("zip")) {
      std::string file_path = vm["zip"].as<std::string>();
      oko::CacheDirectoriesManager cache_manager;
      if (!cache_manager.is_initialized()) {
        std::cerr << "Failed initialize cache" << std::endl;
        return 1;
      }
      std::filesystem::path cache_dir =
          cache_manager.DirectoryForFile(file_path);
      if (cache_dir.empty()) {
        std::cerr << "Failed initialize cache entry" << std::endl;
        return 1;
      }
      provider = std::make_unique<oko::ZipArchiveFilesProvider>(
          std::move(cache_dir),
          std::move(file_path));
    } else if (vm.count("s3")) {
      std::string s3_url = vm["s3"].as<std::string>();
      oko::CacheDirectoriesManager cache_manager;
      if (!cache_manager.is_initialized()) {
        std::cerr << "Failed initialize cache" << std::endl;
        return 1;
      }
      std::filesystem::path cache_dir =
          cache_manager.DirectoryForS3Url(s3_url);
      if (cache_dir.empty()) {
        std::cerr << "Failed initialize cache entry" << std::endl;
        return 1;
      }
      provider = std::make_unique<oko::S3LogFilesProvider>(
          std::move(cache_dir),
          std::move(s3_url));
    }
    auto log_paths = RunChooseFile(*provider);
    if (log_paths.empty()) {
      return 1;
    }
    for (const auto& log_path : log_paths) {
      FileWithPath item;
      if (oko::TextLogFile::NameMatches(log_path.filename())) {
        item.file = std::make_unique<oko::TextLogFile>();
      } else if (oko::MemorylogLogFile::NameMatches(log_path.filename())) {
        item.file = std::make_unique<oko::MemorylogLogFile>();
      } else {
        assert(false);
        return 1;
      }
      item.log_path = std::move(log_path);
      files_with_paths.emplace_back(std::move(item));
    }
  } else {
    // Check in code above should exit program in this case.
    assert(false);
    return 1;
  }
  std::future<bool> parse_async = std::async(
      std::launch::async,
      [&files_with_paths] {
        for (const auto& [file, log_path] : files_with_paths) {
          if (!file->Parse(log_path)) {
            return false;
          }
        }
        return true;
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
  if (bool parse_success = parse_async.get(); !parse_success) {
    std::cerr << "Failed parse file" << std::endl;
    return 1;
  }
  std::vector<std::unique_ptr<oko::LogFile>> files(files_with_paths.size());
  std::transform(
      files_with_paths.begin(),
      files_with_paths.end(),
      files.begin(),
      [](FileWithPath& f) {
        return std::move(f.file);
      });
  ShowFiles(std::move(files));
  return 0;
}
