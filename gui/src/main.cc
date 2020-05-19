/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <getopt.h>

#include <gtk/gtk.h>

#include <list>
#include <string>
#include <iostream>

#ifdef HAVE_BOOST_PROGRAM_OPTIONS_HPP
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#endif

#include <scroom/gtk-helpers.hh>

#include "callbacks.hh"
#include "loader.hh"

#ifdef HAVE_BOOST_PROGRAM_OPTIONS_HPP

void usage(const std::string& me, const po::options_description& desc, const std::string& message=std::string())
{
  if(message.length() != 0)
    std::cout << "ERROR: " << message << std::endl << std::endl;

  std::cout << "Usage: " << me << " [options] [input files]" << std::endl << std::endl;

  std::cout << desc << std::endl;
  exit(-1);
}

#else

void usage(const std::string& me, const std::string& message=std::string())
{
  if(message.length() != 0)
    printf ("ERROR: %s\n\n", message.c_str());

  printf("Usage: %s [options] [input files]\n\n", me.c_str());
  printf("Options:\n");
  printf(" -h            : Show this help\n");
  exit(-1);
}

#endif

int main (int argc, char *argv[])
{
  std::string me = argv[0];
  std::map<std::string, std::list<std::string> > filenames;

#ifdef HAVE_BOOST_PROGRAM_OPTIONS_HPP
  po::options_description desc("Available options");
  desc.add_options()
    ("help,h", "Show this help message")
    ("load,l", po::value<std::vector<std::string> >(), "Load given filenames")
    ("transparent-overlay", po::value<std::vector<std::string> >()->multitoken(), "Show given files in transparent overlay");

  po::positional_options_description p;
  p.add("load", -1);

  po::variables_map vm;

  try
  {
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if(vm.count("help"))
      usage(me, desc);

    if(vm.count("load"))
    {
      const std::vector<std::string>& names = vm["load"].as<std::vector<std::string> >();
      filenames[REGULAR_FILES].assign(names.begin(), names.end());
    }

    if(vm.count("transparent-overlay"))
    {
      const std::vector<std::string>& names = vm["transparent-overlay"].as<std::vector<std::string> >();
      filenames["Transparent Overlay"].assign(names.begin(), names.end());
    }
  }
  catch(std::exception& ex)
  {
    usage(me, desc, ex.what());
  }

#else
  char result;

  while ((result = getopt(argc, argv, ":h")) != -1)
  {
    switch (result)
    {
    case 'h':
      usage(me);
      break;
    case '?':
      usage(me, "Unknown option");
      break;
    case ':':
      usage(me, "Option requires an argument");
      break;
    default:
      usage(me, "This shouldn't be happening");
      break;
    }
  }

  while(optind < argc)
  {
    filenames[REGULAR_FILES].push_back(std::string(argv[optind]));
    optind++;
  }
#endif

  Scroom::GtkHelpers::useRecursiveGdkLock();
  g_thread_init(NULL);
  gdk_threads_init();

  gdk_threads_enter();
  gtk_set_locale ();
  gtk_init (&argc, &argv);

  on_scroom_bootstrap(filenames);

  gtk_main ();
  gdk_threads_leave();

  printf("DEBUG: Scroom terminating...\n");
  return 0;
}

