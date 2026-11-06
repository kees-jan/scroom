/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <chrono>
#include <future>

#include <gtest/gtest.h>

#include <scroom/layer-stack.hh>
#include <scroom/tiledbitmapinterface.hh>

#include "tile-sample-helpers.hh"

using namespace Scroom::TiledBitmap;
using namespace Scroom::TiledBitmap::Test;
using Scroom::Utils::Point;
using Scroom::Utils::Rectangle;

namespace
{
  constexpr int reductionFactor = 8;
  constexpr auto completionTimeout = std::chrono::seconds(10);

  class PopulationProgress : public ProgressInterface
  {
  private:
    std::promise<void> m_finished;

  public:
    [[nodiscard]] std::future<void> finished() { return m_finished.get_future(); }

    void setIdle() override {}
    void setWaiting(double /*progress*/) override {}
    void setWorking(double /*progress*/) override {}
    void setFinished() override { m_finished.set_value(); }
  };

  class SourceWithContrastingLeftStrip : public SourcePresentation
  {
  private:
    unsigned m_leftStripValue;
    unsigned m_backgroundValue;
    std::promise<void> m_readFinished;

  public:
    SourceWithContrastingLeftStrip(unsigned leftStripValue, unsigned backgroundValue)
      : m_leftStripValue(leftStripValue)
      , m_backgroundValue(backgroundValue)
    {
    }

    [[nodiscard]] std::future<void> readFinished() { return m_readFinished.get_future(); }

    void fillTiles(int /*startLine*/, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles) override
    {
      const SampleLayout format{8, 1};
      const Rectangle populatedArea(0, 0, tileWidth, lineCount);
      for(const auto& tile: tiles)
      {
        fillChannel(tile, format, populatedArea, 0, m_backgroundValue);
      }
      if(firstTile == 0)
      {
        fillChannel(tiles.front(), format, populatedArea.leftOf(reductionFactor), 0, m_leftStripValue);
      }
    }

    void done() override { m_readFinished.set_value(); }
    std::string getName() override { return "contrasting left strip"; }
  };
} // namespace

TEST(LayerStack, exposes_layer_geometry_and_storage_precision_without_rendering_operations)
{
  const Point imageSize(3 * TILESIZE, reductionFactor);
  const LayerSpec descriptions{
    LayerDescription(1, createIntensityReduction(1), {}), LayerDescription(8, createIntensityReduction(8), {})
  };
  const auto stack = LayerStack::create(imageSize, descriptions);
  const auto levels = stack->getLevels();

  ASSERT_GE(levels.size(), 2U);
  EXPECT_EQ(1, levels.front().bpp);
  EXPECT_EQ(8, levels[1].bpp);
  EXPECT_EQ(1, levels.front().layer->getTile({0, 0})->bpp);
  EXPECT_EQ(8, levels[1].layer->getTile({0, 0})->bpp);
  const Rectangle imageArea(0, 0, imageSize.x, imageSize.y);
  EXPECT_EQ(imageArea, levels.front().layer->getRect());
  EXPECT_EQ(imageArea / reductionFactor, levels[1].layer->getRect());
}

TEST(LayerStack, loads_and_reduces_source_pixels_without_a_view_or_main_loop)
{
  const unsigned leftStripValue = 20;
  const unsigned backgroundValue = 60;
  const LayerSpec descriptions{LayerDescription(8, createIntensityReduction(8), {})};
  const auto stack = LayerStack::create(Point(3 * TILESIZE, reductionFactor), descriptions);
  ASSERT_GE(stack->getLevels().size(), 2U);
  const auto progress = std::make_shared<PopulationProgress>();
  auto populated = progress->finished();
  const auto subscription = stack->subscribeProgress(progress);
  const auto source = std::make_shared<SourceWithContrastingLeftStrip>(leftStripValue, backgroundValue);
  auto readFinished = source->readFinished();

  stack->setSource(source);

  ASSERT_EQ(std::future_status::ready, readFinished.wait_for(completionTimeout));
  ASSERT_EQ(std::future_status::ready, populated.wait_for(completionTimeout));
  const SampleLayout format{8, 1};
  const auto bottom = stack->getLevels().front().layer->getTile({0, 0})->getTileSync();
  const auto reduced = stack->getLevels()[1].layer->getTile({0, 0})->getTileSync();
  EXPECT_EQ(leftStripValue, sample(bottom, format, 0, 0));
  EXPECT_EQ(backgroundValue, sample(bottom, format, reductionFactor, 0));
  EXPECT_EQ(leftStripValue, sample(reduced, format, 0, 0));
  EXPECT_EQ(backgroundValue, sample(reduced, format, 1, 0));
  const Point secondSourceTileInReducedLayer(TILESIZE / reductionFactor, 0);
  EXPECT_EQ(backgroundValue, sample(reduced, format, secondSourceTileInReducedLayer));
}

TEST(LayerStack, releasing_the_stack_releases_its_layers_and_reduction_wiring)
{
  const LayerSpec descriptions{LayerDescription(8, createIntensityReduction(8), {})};
  auto stack = LayerStack::create(Point(3 * TILESIZE, reductionFactor), descriptions);
  ASSERT_GE(stack->getLevels().size(), 2U);
  const std::weak_ptr<Layer> bottom = stack->getLevels().front().layer;
  const std::weak_ptr<Layer> reduced = stack->getLevels()[1].layer;

  stack.reset();

  EXPECT_TRUE(bottom.expired());
  EXPECT_TRUE(reduced.expired());
}

TEST(LayerStack, owns_the_injected_load_lifetime_token)
{
  const LayerSpec descriptions{LayerDescription(8, createIntensityReduction(8), {})};
  auto bitmap = createTiledBitmap(reductionFactor, reductionFactor, descriptions);
  auto stack = bitmap->getLayerStack();
  EXPECT_EQ(bitmap->getBottomLayer(), stack->getLevels().front().layer);
  auto loadLifetimeToken = std::make_shared<int>();
  const std::weak_ptr<int> weakLoadLifetimeToken = loadLifetimeToken;

  stack->startLoading([loadLifetimeToken](const ProgressInterface::Ptr& /*progress*/) { return loadLifetimeToken; });
  loadLifetimeToken.reset();
  bitmap.reset();

  EXPECT_FALSE(weakLoadLifetimeToken.expired());
  stack.reset();
  EXPECT_TRUE(weakLoadLifetimeToken.expired());
}
