/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
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
#include <scroom/showmetadata.hh>
#include <scroom/tiledbitmaplayer.hh>

namespace Scroom::TiledBitmap
{
  /**
   * Function type for starting the loading of the bottom Layer of a bitmap.
   *
   * This function is called on the UI thread. You cannot do a significant amount of work in this function, or you'll block the
   * UI. Typical implementations will schedule work on the Sequentially() ThreadPool, such that bitmaps will be loaded
   * sequentially.
   *
   * @param progressInterface call setProgress(0) on this once the work actually starts
   * @return a shared pointer. Typically a ThreadPool::Queue or similar. The expectation is that when this object is destroyed
   * by the caller, the loading is cancelled.
   *
   * @see Layer
   * @see OpenTiledBitmapInterface::open()
   * @see Sequentially()
   * @see scheduleLoadingBitmap() for an example implementation
   */
  using ReloadFunction = std::function<Scroom::Utils::Stuff(const ProgressInterface::Ptr&)>;

  struct BitmapMetaData
  {
    std::string                   type;
    unsigned int                  bitsPerSample;
    unsigned int                  samplesPerPixel;
    Scroom::Utils::Rectangle<int> rect; /**< size & offset, in pixels, excluding any deformation by @c aspectRatio */
    boost::optional<Scroom::Utils::Point<double>> aspectRatio;
    ColormapHelperBase::Ptr                       colormapHelper;
  };

  std::ostream&      to_stream(std::ostream& os, const BitmapMetaData& bmd);
  std::string        to_string(const BitmapMetaData& bmd);
  Metadata::Metadata to_metadata(const BitmapMetaData& bmd);

  class OpenTiledBitmapInterface : private Interface
  {
  public:
    using Ptr = boost::shared_ptr<OpenTiledBitmapInterface>;

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

  using LayerSpecResult        = std::tuple<LayerSpec, ColormapHelperBase::Ptr>;
  using LayerSpecForBitmapFunc = std::function<LayerSpecResult(const BitmapMetaData& bitmapMetaData)>;

  LayerSpecResult LayerSpecForBitmap(const BitmapMetaData& bitmapMetaData);

  Scroom::Utils::Stuff
    scheduleLoadingBitmap(const SourcePresentation::Ptr& sp, const Layer::Ptr& layer, const ProgressInterface::Ptr& progress);

} // namespace Scroom::TiledBitmap
