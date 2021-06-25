/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cstdlib>
#include <iostream>
#include <list>
#include <string>

#include <boost/program_options.hpp>

#include <gtk/gtk.h>

#include <scroom/gtk-helpers.hh>

#include "callbacks.hh"

namespace po = boost::program_options;

void usage(const std::string& me, const po::options_description& desc, const std::string& message = std::string())
{
  if(message.length() != 0)
  {
    std::cout << "ERROR: " << message << std::endl << std::endl;
  }

  std::cout << "Usage: " << me << " [options] [input files]" << std::endl << std::endl;

  std::cout << desc << std::endl;
  exit(-1);
}

int main(int argc, char* argv[])
{
  std::string                                   me = argv[0];
  std::map<std::string, std::list<std::string>> filenames;

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

  Scroom::GtkHelpers::useRecursiveGdkLock();
  gdk_threads_init();

  gdk_threads_enter();
  setlocale(LC_ALL, "");
  gtk_init(&argc, &argv);

  on_scroom_bootstrap(filenames);

  gtk_main();
  gdk_threads_leave();

  on_scroom_terminating();
  printf("DEBUG: Scroom terminating...\n");
  return 0;
}
