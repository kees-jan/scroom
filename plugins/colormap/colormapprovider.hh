/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
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
