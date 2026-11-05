/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cairo.h>

#include <scroom/interface.hh>
#include <scroom/rectangle.hh>
#include <scroom/stuff.hh>
#include <scroom/tile.hh>

class LayerRenderingOperations : private Interface
{
public:
  using Ptr = std::shared_ptr<LayerRenderingOperations>;

public:
  /**
   * Initialize the canvas for drawing the bitmap
   *
   * When TiledBitmapInterface::redraw() is called, (a portion of) the
   * Layer needs to be redrawn. TiledBitmap will compute which tiles
   * are involved in the redraw, and call draw() or drawState() for
   * each of them, as appropriate. However, before doing so, it will
   * first call initializeCairo(). You can take this opportunity to
   * set various properties that are needed in all subsequent calls to
   * draw() and drawState(), such as anti-aliasing and line caps.
   */
  virtual void initializeCairo(cairo_t* cr) = 0;

  /**
   * Draw the given tileArea into the given viewArea at the requested zoom level
   *
   * @param cr The canvas on which to draw
   * @param tile The tile to take data from
   * @param tileArea Area of the tile that needs to be drawn
   * @param viewArea Area of the canvas that the tile needs to be drawn in
   * @param zoom The requested zoom level. One pixel of your
   *    presentation should have size 2**@c zoom when drawn. @c zoom
   *    will be negative for all but the first layer.
   * @param cache Depending on whether the cacheZoom() function finished
   *    already, this may either be an empty reference, or a reference
   *    to the value returned by cacheZoom()
   */
  virtual void draw(
    cairo_t* cr,
    const ConstTile::Ptr& tile,
    Scroom::Utils::Rectangle<double> tileArea,
    Scroom::Utils::Rectangle<double> viewArea,
    int zoom,
    Scroom::Utils::Stuff cache
  ) = 0;

  /**
   * Draw the given state into the given viewArea
   *
   * The associated tile is likely not loaded or not initialized or in
   * whatever other TileState that doesn't allow its contents to be
   * drawn.
   *
   * Something needs to be drawn in the given @c viewArea
   * anyway. Implementors might want to just draw an empty rectangle,
   * maybe color-coded to reflect the state of the tile.
   */
  virtual void drawState(cairo_t* cr, TileState s, Scroom::Utils::Rectangle<double> viewArea) = 0;

  /**
   * Cache data for use during later cacheZoom() calls.
   *
   * This function is called for a given tile, and then the results
   * are passed to the cacheZoom() function. Because draw() is called
   * relatively frequently (i.e. when scrolling), it is recommended to
   * do cpu-intensive work in the cache() and cacheZoom() functions,
   * and then re-use the data to make the draw() faster.
   *
   * The default implementation returns an empty reference, meaning
   * nothing is cached. As a result, the draw() function will receive
   * an empty reference.
   *
   * @param tile the Tile for which caching is requested
   */
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr& /*tile*/) { return {}; }

  /**
   * Cache data for use during later draw() calls.
   *
   * This function is called for a given tile and zoom level, and then
   * the results are passed to the draw() function. Because draw() is
   * called relatively frequently (i.e. when scrolling), it is
   * recommended to do cpu-intensive work in the cache() and
   * cacheZoom() functions, and then re-use the data to make the
   * draw() faster.
   *
   * The default implementation returns an empty reference, meaning
   * nothing is cached. As a result, the draw() function will receive
   * an empty reference.
   *
   * @param tile the Tile for which caching is requested
   * @param zoom the requested zoom level
   * @param cache the output of cache(const ConstTile::Ptr)
   */
  virtual Scroom::Utils::Stuff cacheZoom(const ConstTile::Ptr& /*tile*/, int /*zoom*/, Scroom::Utils::Stuff& /*cache*/)
  {
    return {};
  }
};
