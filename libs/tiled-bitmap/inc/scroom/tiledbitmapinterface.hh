/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <cairo.h>

#include <scroom/context.hh>
#include <scroom/interface.hh>
#include <scroom/layer-reduction.hh>
#include <scroom/layer-rendering-operations.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/rectangle.hh>
#include <scroom/scroominterface.hh>
#include <scroom/stuff.hh>
#include <scroom/tile.hh>
#include <scroom/viewinterface.hh>

// Compatibility facade for existing operation factories. Data consumers retain
// the independent reducer; renderers retain only LayerRenderingOperations.
class LayerOperations : public LayerRenderingOperations
{
public:
  using Ptr = std::shared_ptr<LayerOperations>;

  [[nodiscard]] virtual int getBpp() const = 0;
  [[nodiscard]] virtual Scroom::TiledBitmap::LayerReduction createReduction() const = 0;
};

// The supplier ensures each reducer's output matches the following layer's
// storage size and renderer. Pixel encodings remain the supplier's responsibility.
struct LayerDescription
{
  int bpp;
  Scroom::TiledBitmap::LayerReduction reduction;
  LayerRenderingOperations::Ptr rendering;

  LayerDescription(int bpp_, Scroom::TiledBitmap::LayerReduction reduction_, LayerRenderingOperations::Ptr rendering_)
    : bpp(bpp_)
    , reduction(std::move(reduction_))
    , rendering(std::move(rendering_))
  {
  }

  explicit LayerDescription(LayerOperations::Ptr operations)
    : bpp(operations->getBpp())
    , reduction(operations->createReduction())
    , rendering(std::move(operations))
  {
  }
};

/**
 * Layer descriptions defining how to draw and pre-scale a
 * TiledBitmap
 *
 * While creating a TiledBitmap, a number of layers will be
 * created. The base layer will contain raw bitmap data. Each
 * subsequent layer is reduced by a factor 8 with respect to the
 * previous. Each description supplies bits per pixel and independent reduction and rendering
 * operations. If more layers are created than there are descriptions, the last
 * description is reused for all remaining layers.
 */
using LayerSpec = std::vector<LayerDescription>;

/**
 * Provide bitmap data
 */
class SourcePresentation : private Interface
{
public:
  using Ptr = std::shared_ptr<SourcePresentation>;

public:
  /**
   * Provide bitmap data
   *
   * While the TiledBitmap is loaded, this function will be called
   * several times. Each time, this function has to fill the provided
   * tiles with bitmap data.
   *
   * The given tiles are horizontally adjacent.
   *
   * @param startLine @c y coordinate of the first line in the set of
   *    tiles
   * @param lineCount The number of lines that are to be filled. This
   *    equals the height of the tiles
   * @param tileWidth The width of one tile
   * @param firstTile Index of the first tile provided. This means that
   *    the first @c x coordinate is @c firsttile * @c tileWidth
   * @param tiles The tiles that are to be filled.
   */
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) = 0;

  /**
   * Done reading bitmap data
   *
   * Any open files can be closed now
   */
  virtual void done() = 0;

  virtual std::string getName() = 0;
};

class Layer;

/**
 * Interact with your TiledBitmap
 */
class TiledBitmapInterface : public Viewable
{
public:
  using Ptr = std::shared_ptr<TiledBitmapInterface>;

  /**
   * Provide bitmap data to the TiledBitmap
   *
   * Bitmap data will be loaded and pre-scaled in a fashion that least
   * loads cpu and memory.
   *
   * @param sp source of the bitmap data
   */
  virtual void setSource(SourcePresentation::Ptr sp) = 0;

  /**
   * Retrieve the bottom layer of the TiledBitmap.
   *
   * This allows you to fill the layer with data yourself, instead of relying on setSource()
   *
   * @see setSource()
   */
  virtual std::shared_ptr<Layer> getBottomLayer() = 0;

  /**
   * Redraw a portion of the bitmap.
   *
   * This is typically called from PresentationInterface::redraw()
   *
   * @see PresentationInterface::redraw()
   */
  virtual void
    redraw(ViewInterface::Ptr const& vi, cairo_t* cr, const Scroom::Utils::Rectangle<double>& presentationArea, int zoom) = 0;

  /**
   * Clear all bitmap caches related to the view.
   *
   * You'd typically do this if the bitmap has somehow changed
   * appearance, for example when switching to a new Colormap.
   *
   * @param vi The ViewInterface for which to clear the caches
   */
  virtual void clearCaches(ViewInterface::Ptr vi) = 0;

  [[nodiscard]] virtual Scroom::Utils::Context::ConstPtr getContext() const = 0;
};

/**
 * Create a tiled bitmap.
 *
 * In a TiledBitmap, your bitmap is distributed over tiles, that each
 * contain a square portion of your bitmap. These tiles will be
 * swapped in or out as required, depending on the amount of available
 * memory. This way, you can "load" bitmaps that are larger than your
 * amount of memory would normally allow.
 *
 * Of course, when zooming out, you'd still need all the data in your
 * bitmap, in order to show the scaled down version. To avoid this,
 * ::createTiledBitmap() creates pre-scaled versions of your bitmap,
 * that are reduced by a factor 8. Such a pre-scaled version is called
 * a Layer. If your bitmap gets very large, several pre-scaled
 * versions are created, reduced by a factor 8, 64, 512, 4096, etc.
 *
 * So, to summarize, ::createTiledBitmap() creates several layers of your
 * bitmap, at zoom levels 1:1, 1:8, 1:64, etc. Each layer is then
 * divided into tiles, such that only the relevant part can be loaded
 * into memory.
 *
 * Supply layer descriptions with independent reduction and rendering operations.
 * Existing LayerOperations factories can also be used to construct descriptions.
 *
 * @note When calling ::createTiledBitmap(), all the internal data
 * structures will be set up. You still have to fill your bitmap with
 * data, for example by calling TiledBitmapInterface::setSource()
 *
 * @param bitmapWidth the width of the bitmap
 * @param bitmapHeight the height of the bitmap
 * @param ls Layer descriptions specifying bits per pixel, reduction, and rendering
 *
 * @return A pointer to a newly created TiledBitmapInterface. You can
 *    use this pointer to manipulate your bitmap.
 *
 */
TiledBitmapInterface::Ptr createTiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec const& ls);
