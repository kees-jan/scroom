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

#include <string>

#include <scroom/unused.hh>

#include "measure-framerate-callbacks.hh"
#include "measure-framerate-tests.hh"

void usage(std::string me, std::string message=std::string())
{
  if(message.length() != 0)
    printf ("ERROR: %s\n\n", message.c_str());

  printf("Usage: %s [options] [input files]\n\n", me.c_str());
  printf("Options:\n");
  printf(" -h            : Show this help\n");
  exit(-1);
}

int main (int argc, char *argv[])
{
  std::string me = argv[0];
  int result;

  while ((result = getopt(argc, argv, ":h")) != -1)
  {
    switch (static_cast<char>(result))
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

  gdk_threads_init();

  gdk_threads_enter();
  gtk_set_locale ();
  gtk_init (&argc, &argv);

  GtkWidget* window = create_window();
  UNUSED(window);

  init_tests();
  init();

  gtk_main ();
  gdk_threads_leave();
  return 0;
}

