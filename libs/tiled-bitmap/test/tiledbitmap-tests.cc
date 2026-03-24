/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gtest/gtest.h>

#include <scroom/rectangle.hh>
#include <scroom/tiledbitmapinterface.hh>

//////////////////////////////////////////////////////////////

class DummyLayerOperations : public LayerOperations
{
public:
  static Ptr create() { return Ptr(new DummyLayerOperations()); }

  int getBpp() override { return 8; }
  void initializeCairo(cairo_t* /*cr*/) override {}
  void draw(
    cairo_t* /*cr*/,
    const ConstTile::Ptr& /*tile*/,
    Scroom::Utils::Rectangle<double> /*tileArea*/,
    Scroom::Utils::Rectangle<double> /*viewArea*/,
    int /*zoom*/,
    Scroom::Utils::Stuff /*cache*/
  ) override
  {
  }
  void drawState(cairo_t* /*cr*/, TileState /*s*/, Scroom::Utils::Rectangle<double> /*viewArea*/) override {}
  void reduce(Tile::Ptr /*target*/, const ConstTile::Ptr /*source*/, int /*x*/, int /*y*/) override {}
};

//////////////////////////////////////////////////////////////

TEST(TiledBitmap_Tests, tiledbitmap_can_be_deleted) // NOLINT
{
  LayerSpec ls;
  ls.push_back(DummyLayerOperations::create());
  TiledBitmapInterface::Ptr bitmap = createTiledBitmap(300000, 300000, ls);
  EXPECT_TRUE(bitmap != nullptr);
  std::weak_ptr<TiledBitmapInterface> const weak = bitmap;
  EXPECT_TRUE(weak.lock() != nullptr);
  bitmap.reset();
  EXPECT_FALSE(bitmap != nullptr);
  EXPECT_FALSE(weak.lock() != nullptr);
}
