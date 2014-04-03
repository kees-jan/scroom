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

#ifndef COLORMAPS_HH
#define COLORMAPS_HH

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
       * Load a colormap by name
       */
      Colormap::Ptr load(const char* name);
    };

  }
}
#endif
