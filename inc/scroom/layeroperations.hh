/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/tiledbitmapinterface.hh>
#include <scroom/colormappable.hh>

class CommonOperations : public LayerOperations
{
protected:
  ColormapProvider::Ptr colormapProvider;

public:
  CommonOperations(ColormapProvider::Ptr colormapProvider);
  
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
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  Operations1bpp(ColormapProvider::Ptr colormapProvider);
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
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  Operations8bpp(ColormapProvider::Ptr colormapProvider);
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
  static Ptr create(ColormapProvider::Ptr colormapProvider, int bpp);
  Operations(ColormapProvider::Ptr colormapProvider, int bpp);
  
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
  static Ptr create(ColormapProvider::Ptr colormapProvider, int bpp);
  OperationsColormapped(ColormapProvider::Ptr colormapProvider, int bpp);

  virtual ~OperationsColormapped()
  {}

  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);
};



