/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gtest/gtest.h>

#include <scroom/layer-reduction.hh>
#include <scroom/rectangle.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/tiledbitmaplayer.hh>

//////////////////////////////////////////////////////////////

class DummyLayerOperations : public LayerOperations
{
public:
  static Ptr create() { return Ptr(new DummyLayerOperations()); }

  [[nodiscard]] int getBpp() const override { return 8; }
  [[nodiscard]] Scroom::TiledBitmap::LayerReduction createReduction() const override
  {
    return Scroom::TiledBitmap::createIntensityReduction(8);
  }
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
};

//////////////////////////////////////////////////////////////

TEST(TiledBitmap_Tests, tiledbitmap_can_be_deleted) // NOLINT
{
  LayerSpec ls;
  ls.emplace_back(DummyLayerOperations::create());
  const int largeImageSide = 300000;
  auto bitmap = createTiledBitmap(largeImageSide, largeImageSide, ls);
  ASSERT_NE(nullptr, bitmap);
  const std::weak_ptr<TiledBitmapInterface> weak = bitmap;

  bitmap.reset();

  EXPECT_TRUE(weak.expired());
}

class PluginLayerDescription : public testing::Test
{
protected:
  static constexpr int bytesPerPixel = 5;
  static constexpr int pluginBpp = bytesPerPixel * 8;
  Scroom::TiledBitmap::LayerReduction reduceLayer = [](const Tile::Ptr&, const ConstTile::Ptr&, int, int) {};
  LayerRenderingOperations::Ptr renderLayer = DummyLayerOperations::create();
  LayerSpec descriptions{LayerDescription(pluginBpp, reduceLayer, renderLayer)};
};

TEST_F(PluginLayerDescription, storage_size_is_supplied_independently_of_the_builtin_encodings) // NOLINT
{
  const auto bitmap = createTiledBitmap(1, 1, descriptions);
  EXPECT_EQ(pluginBpp, bitmap->getBottomLayer()->getTile(0, 0)->bpp);
}
