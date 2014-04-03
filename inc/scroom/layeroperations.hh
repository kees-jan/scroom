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

#ifndef _LAYEROPERATIONS_HH
#define _LAYEROPERATIONS_HH

#include <scroom/tiledbitmapinterface.hh>
#include <scroom/colormappable.hh>

class CommonOperations : public LayerOperations
{
protected:
  ColormapProvider* colormapProvider;

public:
  CommonOperations(ColormapProvider* colormapProvider);
  
  virtual ~CommonOperations()
  {}

  void setClip(cairo_t* cr, int x, int y, int width, int height);
  void setClip(cairo_t* cr, const GdkRectangle& area);
  void drawPixelValue(cairo_t* cr, int x, int y, int size, int value);
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual void initializeCairo(cairo_t* cr);
  virtual void drawState(cairo_t* cr, TileState s, GdkRectangle viewArea);

  virtual Scroom::Utils::Stuff cacheZoom(const ConstTile::Ptr tile, int zoom,
                                                Scroom::Utils::Stuff cache);
  virtual void draw(cairo_t* cr, const ConstTile::Ptr tile,
                    GdkRectangle tileArea, GdkRectangle viewArea, int zoom,
                    Scroom::Utils::Stuff cache);
};

class Operations1bpp : public CommonOperations
{
public:
  static Ptr create(ColormapProvider* colormapProvider);
  Operations1bpp(ColormapProvider* colormapProvider);
  virtual ~Operations1bpp()
  {}
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);

  virtual void draw(cairo_t* cr, const ConstTile::Ptr tile,
                    GdkRectangle tileArea, GdkRectangle viewArea, int zoom,
                    Scroom::Utils::Stuff cache);
};

class Operations8bpp : public CommonOperations
{
public:
  static Ptr create(ColormapProvider* colormapProvider);
  Operations8bpp(ColormapProvider* colormapProvider);
  virtual ~Operations8bpp()
  {}
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);

  virtual void draw(cairo_t* cr, const ConstTile::Ptr tile,
                    GdkRectangle tileArea, GdkRectangle viewArea, int zoom,
                    Scroom::Utils::Stuff cache);
};

class Operations : public CommonOperations
{
protected:
  const unsigned bpp;
  const unsigned pixelsPerByte;
  const unsigned pixelOffset;
  const unsigned pixelMask;

public:
  static Ptr create(ColormapProvider* colormapProvider, int bpp);
  Operations(ColormapProvider* colormapProvider, int bpp);
  
  virtual ~Operations()
  {}
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations
  
  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);

  virtual void draw(cairo_t* cr, const ConstTile::Ptr tile,
                    GdkRectangle tileArea, GdkRectangle viewArea, int zoom,
                    Scroom::Utils::Stuff cache);
};

class OperationsColormapped : public Operations
{
public:
  static Ptr create(ColormapProvider* colormapProvider, int bpp);
  OperationsColormapped(ColormapProvider* colormapProvider, int bpp);

  virtual ~OperationsColormapped()
  {}

  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);
};


#endif
