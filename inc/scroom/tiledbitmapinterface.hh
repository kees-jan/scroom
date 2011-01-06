/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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

#ifndef _TILEMANAGERINTERFACE_HH
#define _TILEMANAGERINTERFACE_HH

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <cairo.h>

#include <vector>

#include <boost/shared_ptr.hpp>

#include <scroom/scroominterface.hh>
#include <scroom/viewinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/tile.hh>

/**
 * Represent the state of one of the tiles that make up a layer in the
 * bitmap.
 */
typedef enum
  {
    TILE_UNINITIALIZED, /**< Tile does not yet contain any data */
    TILE_UNLOADED,      /**< Tile does contain data, but has been swapped out */
    TILE_LOADED,        /**< Tile does contain data, and data is in memory */
    TILE_OUT_OF_BOUNDS  /**< Tile is located outside the bitmap area */
  } TileState;

/**
 * Operations on a layer in the TiledBitmap
 *
 * Tasks of LayerOperations objects include:
 * @li Reducing the given Layer by a factor of 8, producing data that
 *    the LayerOperations object of the next layer will understand
 * @li Draw any portion of the given Layer, at the requested zoom level
 *
 * @see Layer
 */
class LayerOperations
{
public:
  virtual ~LayerOperations()
  {
  }

  /**
   * Return the number of bits per pixel that the layer will use.
   *
   * This number will be used to compute the amount of memory required
   * to store one tile
   */
  virtual int getBpp()=0;

  /**
   * Initialize the canvas for drawing the bitmap
   *
   * When TiledBitmapInterface::redraw() is called, (a portion of) the
   * Layer needs to be redrawn. TiledBitmap will compute which tiles
   * are involved in the redraw, and call draw() or drawState() for
   * each of them, as appropriate. However, before doing so, it will
   * first call initializeCairo(). You can take this opportunity to
   * set various properties that are needed in all subsequent calls to
   * draw() and drawStat(), such as antialiasing and line caps.
   */
  virtual void initializeCairo(cairo_t* cr)=0;

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
   */
  virtual void draw(cairo_t* cr, const Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom)=0;

  /**
   * Draw the given state into the given viewArea
   *
   * The associated tile is likely not loaded or not initialised or in
   * whatever other TileState that doesn't allow its contents to be
   * drawn.
   *
   * Something needs to be drawn in the given @c viewArea
   * anyway. Implementors might want to just draw an empty rectangle,
   * maybe color-coded to reflect the state of the tile.
   */
  virtual void drawState(cairo_t* cr, TileState s, GdkRectangle viewArea)=0;

  /**
   * Reduce the source tile by a factor of 8
   *
   * The target tile will contain data for 8*8 source tiles. Offsets
   * @c x and @c y indicate which of those 64 source tiles is
   * currently being processed
   *
   * @param target Tile that will contain the reduced bitmap
   * @param source Tile that is to be reduced
   * @param x x-offset (0..7) of the source tile in the target tile
   * @param y y-offset (0..7) of the source tile in the target tile
   *
   * @note The @c target tile belongs to a different layer, and hence
   *    possibly has a different bpp than the current one, depending
   *    on the ::LayerSpec given to ::createTiledBitmap()
   *
   */
  virtual void reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y)=0;
};

/**
 * Set of LayerOperations, that define how to draw and pre-scale a
 * TiledBitmap
 *
 * While creating a TiledBitmap, a number of layers will be
 * created. The base layer will contain raw bitmap data. Each
 * subsequent layer is reduced by a factor 8 with respect to the
 * previous. For each Layer, a LayerOperations object should be
 * provided. If the TiledBitmap creates more layers than there are
 * LayerOperations objects in the ::LayerSpec, then the last
 * LayerOperations object will be used for all remaining layers.
 */
typedef std::vector<LayerOperations*> LayerSpec;

/**
 * Provide bitmap data
 */
class SourcePresentation
{
public:
  virtual ~SourcePresentation()
  {}

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
  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles)=0;
};

/**
 * Interact with your TiledBitmap
 */
class TiledBitmapInterface: public Viewable
{
public:
  typedef boost::shared_ptr<TiledBitmapInterface> Ptr;
  
  virtual ~TiledBitmapInterface()
  {}

  /**
   * Provide bitmap data to the TiledBitmap
   *
   * Bitmap data will be loaded and pre-scaled in a fashion that least
   * loads cpu and memory.
   *
   * @param sp source of the bitmap data
   */
  virtual void setSource(SourcePresentation* sp)=0;

  /**
   * Redraw a portion of the bitmap.
   *
   * This is typically called from PresentationInterface::redraw()
   *
   * @see PresentationInterface::redraw()
   */
  virtual void redraw(ViewInterface* vi, cairo_t* cr, GdkRectangle presentationArea, int zoom)=0;
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
 * Of course ::createTiledBitmap() doesn't know anything about how you
 * choose to represent your bitmap data, so you'll have to specify (by
 * implementing LayerOperations) how to draw portions of your bitmap
 * at given zoom levels, and how to pre-scale your bitmap.
 *
 * @note When calling ::createTiledBitmap(), all the internal data
 * structures will be set up. You still have to fill your bitmap with
 * data, for example by calling TiledBitmapInterface::setSource()
 *
 * @param bitmapWidth the width of the bitmap
 * @param bitmapHeight the height of the bitmap
 * @param ls LayerOperations objects, specifying how to draw (portions
 *    of) your bitmap at given zoom levels
 *
 * @return A pointer to a newly created TiledBitmapInterface. You can
 *    use this pointer to manipulate your bitmap.
 * 
 */
TiledBitmapInterface::Ptr createTiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls);


#endif
