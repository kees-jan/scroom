/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>

#include <gtk/gtk.h>

#include <list>
#include <string>

#include "callbacks.hh"
#include "loader.hh"

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
  std::list<std::string> filenames;
  char result;

  while ((result = getopt(argc, argv, ":d:f:t:o:c:w:h")) != -1)
  {
    switch (result)
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

  while(optind < argc)
  {
    filenames.push_back(std::string(argv[optind]));
    optind++;
  }

  g_thread_init(NULL);
  gdk_threads_init();
  
  gdk_threads_enter();
  gtk_set_locale ();
  gtk_init (&argc, &argv);

  // scroom = create_scroom ();
  // gtk_widget_show (scroom);

  on_scroom_bootstrap(filenames);
  
  gtk_main ();
  gdk_threads_leave();
  return 0;
}

