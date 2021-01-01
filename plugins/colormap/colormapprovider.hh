/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <map>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <gtk/gtk.h>

#include <scroom/colormappable.hh>
#include <scroom/observable.hh>
#include <scroom/presentationinterface.hh>

namespace Scroom
{
  namespace ColormapImpl
  {
    /**
     * Provide the colormap widget to the Viewable.
     *
     * When the user selects one of the colormaps, they will be set.
     */
    class ColormapProvider
      : public Viewable
      , public boost::enable_shared_from_this<ColormapProvider>
    {
    public:
      using Ptr = boost::shared_ptr<ColormapProvider>;

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
      ~ColormapProvider() override;

      ColormapProvider(const ColormapProvider&) = delete;
      ColormapProvider(ColormapProvider&&)      = delete;
      ColormapProvider operator=(const ColormapProvider&) = delete;
      ColormapProvider operator=(ColormapProvider&&) = delete;

      // Viewable ////////////////////////////////////////////////////////////

      /** A new view was opened */
      void open(ViewInterface::WeakPtr vi) override;

      /** An existing view was closed */
      void close(ViewInterface::WeakPtr vi) override;

      // Helpers /////////////////////////////////////////////////////////////

      /** The user selected a colormap */
      void on_colormap_selected(GtkTreeView* tv);
    };

  } // namespace ColormapImpl
} // namespace Scroom
