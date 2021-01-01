/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/colormappable.hh>
#include <scroom/interface.hh>
#include <scroom/pipettelayeroperations.hh>
#include <scroom/tiledbitmapinterface.hh>

class CommonOperations : public LayerOperations
{
public:
  static void drawPixelValue(cairo_t* cr, int x, int y, int size, int value);
  static void drawPixelValue(cairo_t* cr, int x, int y, int size, int value, Color const& bgColor);

  void initializeCairo(cairo_t* cr) override;
  void drawState(cairo_t* cr, TileState s, Scroom::Utils::Rectangle<double> viewArea) override;

  Scroom::Utils::Stuff cacheZoom(const ConstTile::Ptr tile, int zoom, Scroom::Utils::Stuff cache) override;
  void                 draw(cairo_t*                         cr,
                            const ConstTile::Ptr             tile,
                            Scroom::Utils::Rectangle<double> tileArea,
                            Scroom::Utils::Rectangle<double> viewArea,
                            int                              zoom,
                            Scroom::Utils::Stuff             cache) override;
};

class PipetteCommonOperationsCMYK
  : public PipetteLayerOperations
  , public CommonOperations
{
protected:
  int bps;

public:
  using Ptr = boost::shared_ptr<PipetteCommonOperationsCMYK>;

public:
  PipetteCommonOperationsCMYK(int bps_)
    : bps(bps_){};

  PipetteLayerOperations::PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr tile) override;
};

class PipetteCommonOperationsRGB
  : public PipetteLayerOperations
  , public CommonOperations
{
protected:
  int bps;

public:
  using Ptr = boost::shared_ptr<PipetteCommonOperationsRGB>;

public:
  PipetteCommonOperationsRGB(int bps_)
    : bps(bps_){};

  PipetteLayerOperations::PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr tile) override;
};

class Operations1bpp : public CommonOperations
{
private:
  ColormapProvider::Ptr colormapProvider;

public:
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  Operations1bpp(ColormapProvider::Ptr colormapProvider);

  int                  getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr tile) override;
  void                 reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y) override;

  void draw(cairo_t*                         cr,
            const ConstTile::Ptr             tile,
            Scroom::Utils::Rectangle<double> tileArea,
            Scroom::Utils::Rectangle<double> viewArea,
            int                              zoom,
            Scroom::Utils::Stuff             cache) override;
};

class Operations8bpp : public CommonOperations
{
private:
  ColormapProvider::Ptr colormapProvider;

public:
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  Operations8bpp(ColormapProvider::Ptr colormapProvider);

  int                  getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr tile) override;
  void                 reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y) override;

  void draw(cairo_t*                         cr,
            const ConstTile::Ptr             tile,
            Scroom::Utils::Rectangle<double> tileArea,
            Scroom::Utils::Rectangle<double> viewArea,
            int                              zoom,
            Scroom::Utils::Stuff             cache) override;
};

class Operations24bpp : public PipetteCommonOperationsRGB
{
public:
  static Ptr create();
  Operations24bpp();

  int                  getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr tile) override;
  void                 reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y) override;
};

class Operations : public CommonOperations
{
protected:
  ColormapProvider::Ptr colormapProvider;
  const unsigned        bpp;
  const unsigned        pixelsPerByte;
  const unsigned        pixelOffset;
  const unsigned        pixelMask;

public:
  static Ptr create(ColormapProvider::Ptr colormapProvider, int bpp);
  Operations(ColormapProvider::Ptr colormapProvider, int bpp);

  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  int                  getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr tile) override;
  void                 reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y) override;

  void draw(cairo_t*                         cr,
            const ConstTile::Ptr             tile,
            Scroom::Utils::Rectangle<double> tileArea,
            Scroom::Utils::Rectangle<double> viewArea,
            int                              zoom,
            Scroom::Utils::Stuff             cache) override;
};

class OperationsColormapped : public Operations
{
public:
  static Ptr create(ColormapProvider::Ptr colormapProvider, int bpp);
  OperationsColormapped(ColormapProvider::Ptr colormapProvider, int bpp);

  int                  getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr tile) override;
  void                 reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y) override;
};

class Operations1bppClipped : public CommonOperations
{
private:
  ColormapProvider::Ptr colormapProvider;

public:
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  Operations1bppClipped(ColormapProvider::Ptr colormapProvider);

  int                  getBpp() override;
  Scroom::Utils::Stuff cacheZoom(const ConstTile::Ptr tile, int zoom, Scroom::Utils::Stuff cache) override;

  void reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y) override;
};

class OperationsCMYK32 : public PipetteCommonOperationsCMYK
{
public:
  static Ptr create();
  OperationsCMYK32();

  int                  getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr tile) override;
  void                 reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y) override;
};

class OperationsCMYK16 : public PipetteCommonOperationsCMYK
{
public:
  static Ptr create();
  OperationsCMYK16();

  int                  getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr tile) override;
  void                 reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y) override;
};

class OperationsCMYK8 : public PipetteCommonOperationsCMYK
{
public:
  static Ptr create();
  OperationsCMYK8();

  int                  getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr tile) override;
  void                 reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y) override;
};

class OperationsCMYK4 : public PipetteCommonOperationsCMYK
{
public:
  static Ptr create();
  OperationsCMYK4();

  int                  getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr tile) override;
  void                 reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y) override;
};
