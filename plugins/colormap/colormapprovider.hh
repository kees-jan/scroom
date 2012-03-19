/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

#ifndef COLORMAPPROVIDER_HH
#define COLORMAPPROVIDER_HH

#include <map>

#include <gtk/gtk.h>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <scroom/presentationinterface.hh>
#include <scroom/colormappable.hh>
#include <scroom/observable.hh>

namespace Scroom
{
  namespace ColormapImpl
  {
    /**
     * Provide the colormap widget to the Viewable.
     *
     * When the user selects one of the colormaps, they will be set.
     */
    class ColormapProvider: public Viewable,
        public boost::enable_shared_from_this<ColormapProvider>
    {
    public:
      typedef boost::shared_ptr<ColormapProvider> Ptr;

    private:
      /** The Colormappable interface of the presentation to which we're associated */
      Colormappable::WeakPtr colormappable;

      /** The views to which we're associated */
      std::map<ViewInterface::WeakPtr, GtkTreeView*> views;

      /** The colormaps we're offering to our views */
      GtkListStore* colormaps;

      /** Our registration with the Colormappable */
      Scroom::Utils::Stuff registration;

      /** Constructor */
      ColormapProvider(Colormappable::Ptr c);

    public:
      /** Constructor */
      static ColormapProvider::Ptr create(PresentationInterface::Ptr p);

      /** Destructor */
      ~ColormapProvider();

      // Viewable ////////////////////////////////////////////////////////////

      /** A new view was opened */
      virtual void open(ViewInterface::WeakPtr vi);

      /** An existing view was closed */
      virtual void close(ViewInterface::WeakPtr vi);

      // Helpers /////////////////////////////////////////////////////////////

      /** The user selected a colormap */
      void on_colormap_selected(GtkTreeView* tv);

    };

  }
}

#endif
