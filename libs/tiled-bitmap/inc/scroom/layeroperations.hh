/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstdint>

#include <scroom/colormappable.hh>
#include <scroom/interface.hh>
#include <scroom/pipettelayeroperations.hh>
#include <scroom/tiledbitmapinterface.hh>

class CommonOperations : public LayerOperations
{
public:
  // 65535 == 255 * 257, so dividing by 257 maps [0, 65535] to [0, 255].
  static inline uint8_t channel16To8(uint16_t value) { return static_cast<uint8_t>(value / 257); }

  static void drawPixelValue(cairo_t* cr, int x, int y, int size, int value);
  static void drawPixelValue(cairo_t* cr, int x, int y, int size, int value, Color const& bgColor);

  void initializeCairo(cairo_t* cr) override;
  void drawState(cairo_t* cr, TileState s, Scroom::Utils::Rectangle<double> viewArea) override;

  Scroom::Utils::Stuff cacheZoom(const ConstTile::Ptr& tile, int zoom, Scroom::Utils::Stuff& cache) override;
  void draw(
    cairo_t* cr,
    const ConstTile::Ptr& tile,
    Scroom::Utils::Rectangle<double> tileArea,
    Scroom::Utils::Rectangle<double> viewArea,
    int zoom,
    Scroom::Utils::Stuff cache
  ) override;
};

class PipetteCommonOperationsCMYK
  : public PipetteLayerOperations
  , public CommonOperations
{
protected:
  int bps;

public:
  using Ptr = std::shared_ptr<PipetteCommonOperationsCMYK>;

public:
  explicit PipetteCommonOperationsCMYK(int bps_)
    : bps(bps_) {};

  PipetteLayerOperations::PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr& tile) override;
};

class PipetteCommonOperationsCMYK64bpp
  : public PipetteLayerOperations
  , public CommonOperations
{
public:
  using Ptr = std::shared_ptr<PipetteCommonOperationsCMYK64bpp>;

public:
  PipetteLayerOperations::PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr& tile) override;
};

class PipetteCommonOperationsRGB
  : public PipetteLayerOperations
  , public CommonOperations
{
protected:
  int bps;

public:
  using Ptr = std::shared_ptr<PipetteCommonOperationsRGB>;

public:
  explicit PipetteCommonOperationsRGB(int bps_)
    : bps(bps_) {};

  PipetteLayerOperations::PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr& tile) override;
};

class PipetteCommonOperationsRGB48bpp
  : public PipetteLayerOperations
  , public CommonOperations
{
public:
  using Ptr = std::shared_ptr<PipetteCommonOperationsRGB48bpp>;

public:
  PipetteLayerOperations::PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr& tile) override;
};

class PipetteCommonOperations16bpp
  : public PipetteLayerOperations
  , public CommonOperations
{
public:
  using Ptr = std::shared_ptr<PipetteCommonOperations16bpp>;

public:
  PipetteLayerOperations::PipetteColor sumPixelValues(Scroom::Utils::Rectangle<int> area, const ConstTile::Ptr& tile) override;
};

class Operations1bpp : public CommonOperations
{
private:
  ColormapProvider::Ptr colormapProvider;

public:
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  explicit Operations1bpp(ColormapProvider::Ptr colormapProvider);

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;

  void draw(
    cairo_t* cr,
    const ConstTile::Ptr& tile,
    Scroom::Utils::Rectangle<double> tileArea,
    Scroom::Utils::Rectangle<double> viewArea,
    int zoom,
    Scroom::Utils::Stuff cache
  ) override;
};

class Operations8bpp : public CommonOperations
{
private:
  ColormapProvider::Ptr colormapProvider;

public:
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  explicit Operations8bpp(ColormapProvider::Ptr colormapProvider);

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;

  void draw(
    cairo_t* cr,
    const ConstTile::Ptr& tile,
    Scroom::Utils::Rectangle<double> tileArea,
    Scroom::Utils::Rectangle<double> viewArea,
    int zoom,
    Scroom::Utils::Stuff cache
  ) override;
};

class Operations16bpp : public PipetteCommonOperations16bpp
{
private:
  ColormapProvider::Ptr colormapProvider;

public:
  static LayerOperations::Ptr create(ColormapProvider::Ptr colormapProvider);
  explicit Operations16bpp(ColormapProvider::Ptr colormapProvider);

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;

  void draw(
    cairo_t* cr,
    const ConstTile::Ptr& tile,
    Scroom::Utils::Rectangle<double> tileArea,
    Scroom::Utils::Rectangle<double> viewArea,
    int zoom,
    Scroom::Utils::Stuff cache
  ) override;
};

class OperationsRgb24bpp : public PipetteCommonOperationsRGB
{
public:
  static Ptr create();
  OperationsRgb24bpp();

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;
};

class OperationsRgb48bpp : public PipetteCommonOperationsRGB48bpp
{
public:
  static Ptr create();
  OperationsRgb48bpp();

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;
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

  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;

  void draw(
    cairo_t* cr,
    const ConstTile::Ptr& tile,
    Scroom::Utils::Rectangle<double> tileArea,
    Scroom::Utils::Rectangle<double> viewArea,
    int zoom,
    Scroom::Utils::Stuff cache
  ) override;
};

class OperationsColormapped : public Operations
{
public:
  static Ptr create(ColormapProvider::Ptr colormapProvider, int bpp);
  OperationsColormapped(ColormapProvider::Ptr colormapProvider, int bpp);

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;
};

class Operations1bppClipped : public CommonOperations
{
private:
  ColormapProvider::Ptr colormapProvider;

public:
  static Ptr create(ColormapProvider::Ptr colormapProvider);
  explicit Operations1bppClipped(ColormapProvider::Ptr colormapProvider);

  int getBpp() override;
  Scroom::Utils::Stuff cacheZoom(const ConstTile::Ptr& tile, int zoom, Scroom::Utils::Stuff& cache) override;

  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;
};

class OperationsCMYK32 : public PipetteCommonOperationsCMYK
{
public:
  static Ptr create();
  OperationsCMYK32();

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;
};

class OperationsCMYK64 : public PipetteCommonOperationsCMYK64bpp
{
public:
  static Ptr create();
  OperationsCMYK64();

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;
};

class OperationsCMYK16 : public PipetteCommonOperationsCMYK
{
public:
  static Ptr create();
  OperationsCMYK16();

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;
};

class OperationsCMYK8 : public PipetteCommonOperationsCMYK
{
public:
  static Ptr create();
  OperationsCMYK8();

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;
};

class OperationsCMYK4 : public PipetteCommonOperationsCMYK
{
public:
  static Ptr create();
  OperationsCMYK4();

  int getBpp() override;
  Scroom::Utils::Stuff cache(const ConstTile::Ptr& tile) override;
  void reduce(Tile::Ptr target, ConstTile::Ptr source, int x, int y) override;
};
