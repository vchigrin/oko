// Copyright 2020 The "Oko" project authors. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

void AnalyzeFile(const std::string& log_path) noexcept {
  std::cout << "Analyzing " << log_path << std::endl;
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
