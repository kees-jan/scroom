/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstdlib>
#include <iostream>
#include <list>
#include <string>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <gtk/gtk.h>

#include "callbacks.hh"

namespace po = boost::program_options;

void usage(const std::string& me, const po::options_description& desc, const std::string& message = std::string())
{
  if(message.length() != 0)
  {
    spdlog::error("{}", message);
  }

  spdlog::info("Usage: {}  [options] [input files]", me);

  spdlog::info("{}", boost::lexical_cast<std::string>(desc));

  exit(-1); // NOLINT(concurrency-mt-unsafe)
}

int main(int argc, char* argv[])
{
  std::string                                   me = argv[0]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  std::map<std::string, std::list<std::string>> filenames;

  spdlog::set_level(spdlog::level::debug);

  po::options_description desc("Available options");
  desc.add_options()("help,h", "Show this help message")("load,l", po::value<std::vector<std::string>>(), "Load given filenames")(
    "transparent-overlay", po::value<std::vector<std::string>>()->multitoken(), "Show given files in transparent overlay");

  po::positional_options_description p;
  p.add("load", -1);

  po::variables_map vm;

  try
  {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if(vm.count("help"))
    {
      usage(me, desc);
    }

    if(vm.count("load"))
    {
      const auto& names = vm["load"].as<std::vector<std::string>>();
      filenames[REGULAR_FILES].assign(names.begin(), names.end());
    }

    if(vm.count("transparent-overlay"))
    {
      const auto& names = vm["transparent-overlay"].as<std::vector<std::string>>();
      filenames["Transparent Overlay"].assign(names.begin(), names.end());
    }
  }
  catch(std::exception& ex)
  {
    usage(me, desc, ex.what());
  }

  setlocale(LC_ALL, ""); // NOLINT(concurrency-mt-unsafe)
  gtk_init(&argc, &argv);

  on_scroom_bootstrap(filenames);

  gtk_main();

  on_scroom_terminating();
  spdlog::debug("Scroom terminating...");
  return 0;
}
