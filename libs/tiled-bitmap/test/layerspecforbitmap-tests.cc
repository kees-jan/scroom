/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstring>

#include <gmock/gmock.h>

#include <scroom/bitmap-helpers.hh>
#include <scroom/opentiledbitmapinterface.hh>
#include <scroom/pipettelayeroperations.hh>

#include "tile-sample-helpers.hh"

using namespace Scroom::TiledBitmap;
using namespace Scroom::TiledBitmap::Test;

namespace
{
  constexpr int reductionFactor = 8;
}

class BitmapLayerDescriptions : public testing::Test
{
protected:
  static LayerSpecResult
    descriptionsFor(const std::string& type, unsigned bitsPerSample, unsigned channels, ColormapHelperBase::Ptr colormap = {})
  {
    const BitmapMetaData metadata{
      type, bitsPerSample, channels, {0, 0, reductionFactor, reductionFactor}, {}, std::move(colormap)
    };
    return LayerSpecForBitmap(metadata);
  }
};

TEST_F(BitmapLayerDescriptions, monochrome_storage_expands_to_a_byte_of_intensity) // NOLINT
{
  const int sourceBits = 1;
  const int reducedBits = 8;
  auto [layers, colormap] = descriptionsFor(Greyscale, sourceBits, 1);
  ASSERT_FALSE(layers.empty());
  EXPECT_EQ(sourceBits, layers.front().bpp);
  EXPECT_EQ(reducedBits, layers.back().bpp);
}

TEST_F(BitmapLayerDescriptions, two_and_four_bit_greyscale_keep_two_indices_per_reduced_pixel) // NOLINT
{
  for(const int bitsPerIndex: {2, 4})
  {
    SCOPED_TRACE(bitsPerIndex);
    auto [layers, colormap] = descriptionsFor(Greyscale, bitsPerIndex, 1);
    ASSERT_FALSE(layers.empty());
    EXPECT_EQ(bitsPerIndex, layers.front().bpp);
    EXPECT_EQ(2 * bitsPerIndex, layers.back().bpp);
  }
}

TEST_F(BitmapLayerDescriptions, byte_and_word_greyscale_keep_their_storage_precision) // NOLINT
{
  for(const int bitsPerSample: {8, 16})
  {
    SCOPED_TRACE(bitsPerSample);
    auto [layers, colormap] = descriptionsFor(Greyscale, bitsPerSample, 1);
    ASSERT_FALSE(layers.empty());
    EXPECT_EQ(bitsPerSample, layers.front().bpp);
    EXPECT_EQ(bitsPerSample, layers.back().bpp);
  }
}

TEST_F(BitmapLayerDescriptions, rgb_keeps_three_channels_and_exposes_pipette_operations) // NOLINT
{
  const int channels = RgbChannels::COUNT;
  for(const int bitsPerSample: {8, 16})
  {
    SCOPED_TRACE(bitsPerSample);
    auto [layers, colormap] = descriptionsFor(RGB, bitsPerSample, channels);
    ASSERT_FALSE(layers.empty());
    EXPECT_EQ(channels * bitsPerSample, layers.front().bpp);
    EXPECT_EQ(channels * bitsPerSample, layers.back().bpp);
    EXPECT_NE(nullptr, std::dynamic_pointer_cast<PipetteLayerOperations>(layers.front().rendering));
  }
}

TEST_F(BitmapLayerDescriptions, one_two_and_four_bit_cmyk_channels_expand_to_bytes) // NOLINT
{
  const int channels = CmykChannels::COUNT;
  const int bitsPerReducedChannel = 8;
  for(const int bitsPerSample: {1, 2, 4})
  {
    SCOPED_TRACE(bitsPerSample);
    auto [layers, colormap] = descriptionsFor(CMYK, bitsPerSample, channels);
    ASSERT_FALSE(layers.empty());
    EXPECT_EQ(channels * bitsPerSample, layers.front().bpp);
    EXPECT_EQ(channels * bitsPerReducedChannel, layers.back().bpp);
    EXPECT_NE(nullptr, std::dynamic_pointer_cast<PipetteLayerOperations>(layers.front().rendering));
  }
}

TEST_F(BitmapLayerDescriptions, byte_and_word_cmyk_keep_their_four_channel_precision) // NOLINT
{
  const int channels = CmykChannels::COUNT;
  for(const int bitsPerSample: {8, 16})
  {
    SCOPED_TRACE(bitsPerSample);
    auto [layers, colormap] = descriptionsFor(CMYK, bitsPerSample, channels);
    ASSERT_FALSE(layers.empty());
    EXPECT_EQ(channels * bitsPerSample, layers.front().bpp);
    EXPECT_EQ(channels * bitsPerSample, layers.back().bpp);
    EXPECT_NE(nullptr, std::dynamic_pointer_cast<PipetteLayerOperations>(layers.front().rendering));
  }
}

TEST_F(BitmapLayerDescriptions, indexed_colour_keeps_two_indices_per_reduced_pixel) // NOLINT
{
  for(const int bitsPerIndex: {1, 2, 4, 8})
  {
    SCOPED_TRACE(bitsPerIndex);
    auto suppliedColormap = ColormapHelper::create(1 << bitsPerIndex);
    auto [layers, colormap] = descriptionsFor(Colormapped, bitsPerIndex, 1, suppliedColormap);
    ASSERT_FALSE(layers.empty());
    EXPECT_EQ(bitsPerIndex, layers.front().bpp);
    EXPECT_EQ(2 * bitsPerIndex, layers.back().bpp);
    EXPECT_EQ(suppliedColormap, colormap);
  }
}

TEST_F(BitmapLayerDescriptions, word_greyscale_keeps_its_pipette_capability) // NOLINT
{
  const int bitsPerSample = 16;
  auto [layers, colormap] = descriptionsFor(Greyscale, bitsPerSample, 1);
  ASSERT_FALSE(layers.empty());
  EXPECT_NE(nullptr, std::dynamic_pointer_cast<PipetteLayerOperations>(layers.front().rendering));
}

class ColormappedLayerDescriptions : public testing::Test
{
protected:
  static constexpr unsigned bitsPerIndex = 2;
  ColormapHelperBase::Ptr colormap{ColormapHelper::create(1 << bitsPerIndex)};
  BitmapMetaData metadata{Colormapped, bitsPerIndex, 1, {0, 0, reductionFactor, reductionFactor}, {}, colormap};
  LayerSpec layers{std::get<0>(LayerSpecForBitmap(metadata))};
  const SampleLayout indexFormat{bitsPerIndex, 1};
  const SampleLayout pairFormat{bitsPerIndex, 2};
  Tile::Ptr source{emptyTile(reductionFactor, indexFormat)};
  Tile::Ptr reduced{emptyTile(reductionFactor, pairFormat)};

  static uint32_t cachedPixel(const LayerRenderingOperations::Ptr& renderLayer, const Tile::Ptr& tile)
  {
    auto cache = renderLayer->cache(readOnly(tile));
    const auto surface = std::static_pointer_cast<Scroom::Bitmap::BitmapSurface>(cache);
    uint32_t pixel{};
    memcpy(&pixel, cairo_image_surface_get_data(surface->get()), sizeof(pixel));
    return pixel;
  }
};

TEST_F(ColormappedLayerDescriptions, palette_changes_recolour_both_levels_without_changing_the_indices) // NOLINT
{
  const unsigned chosenIndex = 2;
  fillChannel(source, indexFormat, tileArea(source), 0, chosenIndex);
  fillChannel<uint16_t>(reduced, pairFormat, tileArea(reduced), 0, chosenIndex);
  fillChannel<uint16_t>(reduced, pairFormat, tileArea(reduced), 1, chosenIndex);
  const Color originalColour(1, 0, 0);
  const Color replacementColour(0, 0, 1);
  auto originalPalette = Colormap::createDefault(1 << bitsPerIndex);
  originalPalette->colors[chosenIndex] = originalColour;
  colormap->setColormap(originalPalette);

  EXPECT_EQ(originalColour.getARGB32(), cachedPixel(layers.front().rendering, source));
  EXPECT_EQ(originalColour.getARGB32(), cachedPixel(layers.back().rendering, reduced));

  auto replacementPalette = originalPalette->clone();
  replacementPalette->colors[chosenIndex] = replacementColour;
  colormap->setColormap(replacementPalette);

  EXPECT_EQ(replacementColour.getARGB32(), cachedPixel(layers.front().rendering, source));
  EXPECT_EQ(replacementColour.getARGB32(), cachedPixel(layers.back().rendering, reduced));
  EXPECT_EQ(chosenIndex, sample<uint16_t>(reduced, pairFormat, 0, 0, 0));
  EXPECT_EQ(chosenIndex, sample<uint16_t>(reduced, pairFormat, 0, 0, 1));
}

TEST_F(ColormappedLayerDescriptions, a_reducer_copy_outlives_the_descriptions_and_their_renderers) // NOLINT
{
  const unsigned chosenIndex = 2;
  fillChannel(source, indexFormat, tileArea(source), 0, chosenIndex);
  const auto reduceLayer = layers.front().reduction;
  const std::weak_ptr<LayerRenderingOperations> renderer = layers.front().rendering;
  layers.clear();
  EXPECT_TRUE(renderer.expired());

  reduceLayer(reduced, readOnly(source), 0, 0);

  EXPECT_EQ(chosenIndex, sample<uint16_t>(reduced, pairFormat, 0, 0, 0));
  EXPECT_EQ(chosenIndex, sample<uint16_t>(reduced, pairFormat, 0, 0, 1));
}
