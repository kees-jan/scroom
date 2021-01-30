/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <gtk/gtk.h>

#include <scroom/colormappable.hh>
#include <scroom/interface.hh>
#include <scroom/point.hh>
#include <scroom/rectangle.hh>
#include <scroom/tiledbitmaplayer.hh>

namespace Scroom
{
  namespace TiledBitmap
  {
    using ReloadFunction = std::function<void()>;

    struct BitmapData
    {
      std::string                   type;
      unsigned int                  bitsPerSample;
      unsigned int                  samplesPerPixel;
      Scroom::Utils::Rectangle<int> size; /**< size & offset, in pixels, excluding any deformation by @c aspectRatio */
      Scroom::Utils::Point<double>  aspectRatio;
      Colormap::Ptr                 colormap;
    };

    class OpenTiledBitmapInterface : private Interface
    {
    public:
      using Ptr = boost::shared_ptr<OpenTiledBitmapInterface>;

    public:
      virtual std::list<GtkFileFilter*> getFilters() = 0;

      virtual std::tuple<BitmapData, Layer::Ptr, ReloadFunction> open(const std::string& fileName) = 0;
    };

  } // namespace TiledBitmap
} // namespace Scroom
