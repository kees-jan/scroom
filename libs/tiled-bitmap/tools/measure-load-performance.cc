/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstdlib>
#include <string>

#include <fmt/core.h>
#include <getopt.h>
#include <spdlog/spdlog.h>

#include <gtk/gtk.h>

#include "measure-framerate-callbacks.hh"
#include "measure-load-performance-tests.hh"

void usage(const std::string& me, const std::string& message = std::string())
{
  if(message.length() != 0)
  {
    spdlog::error("{}", message);
  }

  fmt::print("Usage: {} [options] [input files]\n\n", me);
  fmt::print("Options:\n");
  fmt::print(" -h            : Show this help\n");
  exit(-1); // NOLINT(concurrency-mt-unsafe)
}

int main(int argc, char* argv[])
{
  const std::string me = argv[0];
  char              result;

  while((result = getopt(argc, argv, ":h")) != -1)
  {
    switch(result)
    {
    case 'h':
      usage(me);
      break;
    case '?':
      // show usage -- unknown option
      usage(me, "Unknown option");
      break;
    case ':':
      // show usage -- missing argument
      usage(me, "Option requires an argument");
      break;
    default:
      usage(me, "This shouldn't be happening");
      break;
    }
  }

  // while(optind < argc)
  // {
  //   filenames.push_back(std::string(argv[optind]));
  //   optind++;
  // }

  setlocale(LC_ALL, ""); // NOLINT(concurrency-mt-unsafe)
  gtk_init(&argc, &argv);

  init_tests();
  init();

  gtk_main();
  return 0;
}
