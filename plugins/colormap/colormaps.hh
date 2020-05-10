/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>

#include <gtk/gtk.h>

#include<scroom/colormappable.hh>

namespace Scroom
{
  namespace ColormapImpl
  {
    /**
     * Manage the list of colormap files.
     *
     * Upon request, load the selected file, producing a Colormap object.
     *
     * This is a singleton object.
     */
    class Colormaps
    {
    private:
      std::list<Colormap::ConstPtr> colormaps;

    private:
      /** Constructor */
      Colormaps();

      /** Destructor */
      ~Colormaps();

    public:
      /** Get a reference to the instance */
      static Colormaps& getInstance();

      /**
       * Get a copy of the list of Colormap objects.
       */
      std::list<Colormap::ConstPtr> getColormaps();

      /**
       * Get the path where the colormap files are located
       */
      char* getColormapDirPath();

      /**
       * Load a colormap by name
       */
      Colormap::Ptr load(const char* name);
    };

  }
}

