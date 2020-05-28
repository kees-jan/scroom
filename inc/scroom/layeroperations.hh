/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/tiledbitmapinterface.hh>
#include <scroom/colormappable.hh>

class CommonOperations : public LayerOperations
{
public:
  virtual ~CommonOperations()
  {}

  void drawPixelValue(cairo_t* cr, int x, int y, int size, int value);
  void drawPixelValue(cairo_t* cr, int x, int y, int size, int value, Color const& bgColor);

  virtual void initializeCairo(cairo_t* cr);
  virtual void drawState(cairo_t* cr, TileState s, Scroom::Utils::Rectangle<double> viewArea);

  virtual Scroom::Utils::Stuff cacheZoom(const ConstTile::Ptr tile, int zoom,
                                                Scroom::Utils::Stuff cache);
  virtual void draw(cairo_t* cr, const ConstTile::Ptr tile,
                    Scroom::Utils::Rectangle<double> tileArea, Scroom::Utils::Rectangle<double> viewArea, int zoom,
                    Scroom::Utils::Stuff cache);
};

class Operations1bpp : public CommonOperations
{
private:
  ColormapProvider::Ptr colormapProvider;

public:
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  Operations1bpp(ColormapProvider::Ptr colormapProvider);
  virtual ~Operations1bpp()
  {}

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);

  virtual void draw(cairo_t* cr, const ConstTile::Ptr tile,
                    Scroom::Utils::Rectangle<double> tileArea, Scroom::Utils::Rectangle<double> viewArea, int zoom,
                    Scroom::Utils::Stuff cache);
};

class Operations8bpp : public CommonOperations
{
private:
  ColormapProvider::Ptr colormapProvider;

public:
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  Operations8bpp(ColormapProvider::Ptr colormapProvider);
  virtual ~Operations8bpp()
  {}

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);

  virtual void draw(cairo_t* cr, const ConstTile::Ptr tile,
                    Scroom::Utils::Rectangle<double> tileArea, Scroom::Utils::Rectangle<double> viewArea, int zoom,
                    Scroom::Utils::Stuff cache);
};

class Operations24bpp : public CommonOperations
{
public:
  static Ptr create();
  Operations24bpp();
  virtual ~Operations24bpp()
  {}

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);
};

class Operations : public CommonOperations
{
protected:
  ColormapProvider::Ptr colormapProvider;
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
                    Scroom::Utils::Rectangle<double> tileArea, Scroom::Utils::Rectangle<double> viewArea, int zoom,
                    Scroom::Utils::Stuff cache);
};

class OperationsColormapped : public Operations
{
public:
  static Ptr create(ColormapProvider::Ptr colormapProvider, int bpp);
  OperationsColormapped(ColormapProvider::Ptr colormapProvider, int bpp);

  virtual ~OperationsColormapped()
  {}

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);
};

class Operations1bppClipped : public CommonOperations
{
private:
  ColormapProvider::Ptr colormapProvider;

public:
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  Operations1bppClipped(ColormapProvider::Ptr colormapProvider);
  virtual ~Operations1bppClipped()
  {}

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cacheZoom(const ConstTile::Ptr tile, int zoom,
                                         Scroom::Utils::Stuff cache);

  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);
};

class OperationsCMYK32 : public CommonOperations
{
public:
  static Ptr create();
  OperationsCMYK32();
  virtual ~OperationsCMYK32()
  {}

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);
};

class OperationsCMYK16 : public CommonOperations
{
public:
  static Ptr create();
  OperationsCMYK16();
  virtual ~OperationsCMYK16()
  {}

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);
};

class OperationsCMYK8 : public CommonOperations
{
public:
  static Ptr create();
  OperationsCMYK8();
  virtual ~OperationsCMYK8()
  {}

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);
};

class OperationsCMYK4 : public CommonOperations
{
public:
  static Ptr create();
  OperationsCMYK4();
  virtual ~OperationsCMYK4()
  {}

  virtual int getBpp();
  virtual Scroom::Utils::Stuff cache(const ConstTile::Ptr tile);
  virtual void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y);
};
