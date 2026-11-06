/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>

#include <gtk/gtk.h>

#include <scroom/colormappable.hh>
#include <scroom/interface.hh>
#include <scroom/logger.hh>
#include <scroom/point.hh>
#include <scroom/rectangle.hh>
#include <scroom/scroominterface.hh>
#include <scroom/showmetadata.hh>
#include <scroom/tiled-bitmap-data.hh>
#include <scroom/tiledbitmaplayer.hh>

namespace Scroom::TiledBitmap
{

  struct BitmapMetaData
  {
    std::string type;
    unsigned int bitsPerSample;
    unsigned int samplesPerPixel;
    Scroom::Utils::Rectangle<int> rect; /**< size & offset, in pixels, excluding any deformation by @c aspectRatio */
    boost::optional<Scroom::Utils::Point<double>> aspectRatio;
    ColormapHelperBase::Ptr colormapHelper;
  };

  std::ostream& to_stream(std::ostream& os, const BitmapMetaData& bmd);
  std::string to_string(const BitmapMetaData& bmd);
  Metadata::Metadata to_metadata(const BitmapMetaData& bmd);

  class OpenTiledBitmapInterface : private Interface
  {
  public:
    using Ptr = std::shared_ptr<OpenTiledBitmapInterface>;

  public:
    virtual std::list<GtkFileFilter*> getFilters() = 0;

    virtual std::tuple<BitmapMetaData, Layer::Ptr, ReloadFunction> open(const std::string& fileName) = 0;
  };

  OpenPresentationInterface::Ptr ToOpenPresentationInterface(OpenTiledBitmapInterface::Ptr openTiledBitmapInterface);

  ///////////////////////////////////////////////////////////////////

  extern const std::string RGB;
  extern const std::string CMYK;
  extern const std::string Greyscale;
  extern const std::string Colormapped;

  using LayerSpecResult = std::tuple<LayerSpec, ColormapHelperBase::Ptr>;
  using LayerSpecForBitmapFunc =
    std::function<LayerSpecResult(const Scroom::Logger& logger, const BitmapMetaData& bitmapMetaData)>;

  LayerSpecResult LayerSpecForBitmap(const BitmapMetaData& bitmapMetaData);

  Scroom::Utils::Stuff
    scheduleLoadingBitmap(const SourcePresentation::Ptr& sp, const Layer::Ptr& layer, const ProgressInterface::Ptr& progress);

} // namespace Scroom::TiledBitmap
