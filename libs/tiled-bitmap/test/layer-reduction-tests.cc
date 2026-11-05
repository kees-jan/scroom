/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <algorithm>
#include <limits>

#include <gtest/gtest.h>

#include <scroom/layer-reduction.hh>

#include "tile-sample-helpers.hh"

using namespace Scroom::TiledBitmap;
using namespace Scroom::TiledBitmap::Test;

namespace
{
  constexpr int reductionFactor = 8;
}

class Intensity8Reduction : public testing::Test
{
protected:
  // Default to one 8-by-8 reduction block.
  static constexpr int tileSize = reductionFactor;
  const SampleLayout format{8, 1};
  Tile::Ptr source{emptyTile(tileSize, format)};
  Tile::Ptr target{emptyTile(tileSize, format)};
  LayerReduction reduceLayer{createIntensityReduction(format.bitsPerSample)};

  [[nodiscard]] uint8_t targetPixel(int x, int y) const { return sample<uint8_t>(target, format, x, y, 0); }

  void fillSourceArea(Scroom::Utils::Rectangle<int> area, uint8_t value)
  {
    for(int y = area.getTop(); y < area.getBottom(); ++y)
    {
      for(int x = area.getLeft(); x < area.getRight(); ++x)
      {
        setSample<uint8_t>(source, format, x, y, 0, value);
      }
    }
  }
};

TEST_F(Intensity8Reduction, averages_two_equal_halves) // NOLINT
{
  const uint8_t firstHalfValue = 20;
  const uint8_t secondHalfValue = 60;
  const int halfThePixels = tileSize * tileSize / 2;
  std::fill_n(source->data.get(), halfThePixels, firstHalfValue);
  std::fill_n(source->data.get() + halfThePixels, halfThePixels, secondHalfValue);

  reduceLayer(target, readOnly(source), 0, 0);

  EXPECT_EQ((firstHalfValue + secondHalfValue) / 2, targetPixel(0, 0));
}

TEST_F(Intensity8Reduction, places_the_reduced_pixel_at_the_requested_column_and_row) // NOLINT
{
  const uint8_t sourceValue = 80;
  std::fill_n(source->data.get(), tileSize * tileSize, sourceValue);
  const uint8_t untouched = 7;
  std::fill_n(target->data.get(), tileSize * tileSize, untouched);

  const int column = 2;
  const int row = 5;
  reduceLayer(target, readOnly(source), column, row);

  EXPECT_EQ(sourceValue, targetPixel(column, row));
  EXPECT_EQ(untouched, targetPixel(row, column)); // Swapping the axes must not move the pixel here.
  EXPECT_EQ(untouched, targetPixel(column - 1, row));
  EXPECT_EQ(untouched, targetPixel(column + 1, row));
  EXPECT_EQ(untouched, targetPixel(column, row - 1));
  EXPECT_EQ(untouched, targetPixel(column, row + 1));
}

TEST_F(Intensity8Reduction, zero_boundary_rows_count_towards_the_average) // NOLINT
{
  // Only the first row contains image data; the other seven rows remain zero.
  const uint8_t populatedRowValue = 80;
  std::fill_n(source->data.get(), tileSize, populatedRowValue);

  reduceLayer(target, readOnly(source), 0, 0);

  EXPECT_EQ(populatedRowValue / reductionFactor, targetPixel(0, 0));
}

TEST_F(Intensity8Reduction, reduces_four_source_blocks_to_four_pixels_in_their_corresponding_positions) // NOLINT
{
  const int largerTileSize = 2 * reductionFactor;
  source = emptyTile(largerTileSize, format);
  target = emptyTile(largerTileSize, format);

  const uint8_t topLeftValue = 20;
  const uint8_t topRightValue = 60;
  const uint8_t bottomLeftValue = 100;
  const uint8_t bottomRightValue = 140;
  const Scroom::Utils::Rectangle topLeftBlock(0, 0, reductionFactor, reductionFactor);
  const Scroom::Utils::Point oneBlockRight(reductionFactor, 0);
  const Scroom::Utils::Point oneBlockDown(0, reductionFactor);
  fillSourceArea(topLeftBlock, topLeftValue);
  fillSourceArea(topLeftBlock + oneBlockRight, topRightValue);
  fillSourceArea(topLeftBlock + oneBlockDown, bottomLeftValue);
  fillSourceArea(topLeftBlock + oneBlockRight + oneBlockDown, bottomRightValue);

  reduceLayer(target, readOnly(source), 0, 0);

  EXPECT_EQ(topLeftValue, targetPixel(0, 0));
  EXPECT_EQ(topRightValue, targetPixel(1, 0));
  EXPECT_EQ(bottomLeftValue, targetPixel(0, 1));
  EXPECT_EQ(bottomRightValue, targetPixel(1, 1));
}

class Intensity16Reduction : public testing::Test
{
protected:
  const SampleLayout format{16, 1};
  Tile::Ptr source{emptyTile(reductionFactor, format)};
  Tile::Ptr target{emptyTile(reductionFactor, format)};
  LayerReduction reduceLayer{createIntensityReduction(format.bitsPerSample)};
};

TEST_F(Intensity16Reduction, averages_values_that_do_not_fit_in_a_byte) // NOLINT
{
  const unsigned firstHalfValue = 1000;
  const unsigned secondHalfValue = 3000;
  const int halfHeight = source->height / 2;
  const Scroom::Utils::Rectangle topHalf(0, 0, source->width, halfHeight);
  const auto bottomHalf = topHalf + Scroom::Utils::Point(0, halfHeight);
  fillChannel(source, format, topHalf, 0, firstHalfValue);
  fillChannel(source, format, bottomHalf, 0, secondHalfValue);

  reduceLayer(target, readOnly(source), 0, 0);

  EXPECT_EQ((firstHalfValue + secondHalfValue) / 2, firstSample(target, format));
}

class BinaryIntensityReduction : public testing::Test
{
protected:
  const SampleLayout sourceFormat{1, 1};
  const SampleLayout targetFormat{8, 1};
  Tile::Ptr source{emptyTile(reductionFactor, sourceFormat)};
  Tile::Ptr target{emptyTile(reductionFactor, targetFormat)};
  LayerReduction reduceLayer{createIntensityReduction(sourceFormat.bitsPerSample)};
};

TEST_F(BinaryIntensityReduction, half_the_source_bits_set_produces_half_the_byte_intensity) // NOLINT
{
  const unsigned setBit = 1;
  const unsigned fullIntensity = std::numeric_limits<uint8_t>::max();
  const Scroom::Utils::Rectangle topHalf(0, 0, source->width, source->height / 2);
  fillChannel(source, sourceFormat, topHalf, 0, setBit);

  reduceLayer(target, readOnly(source), 0, 0);

  EXPECT_EQ(fullIntensity / 2, firstSample(target, targetFormat));
}

class RgbReduction : public testing::TestWithParam<int>
{
protected:
  const SampleLayout format{GetParam(), RgbChannels::COUNT};
  Tile::Ptr source{emptyTile(reductionFactor, format)};
  Tile::Ptr target{emptyTile(reductionFactor, format)};
  LayerReduction reduceLayer{createRgbReduction(format.bitsPerSample)};

  [[nodiscard]] unsigned targetChannel(int channel) const { return sample(target, format, 0, 0, channel); }
};

INSTANTIATE_TEST_SUITE_P(SampleWidths, RgbReduction, testing::Values(8, 16));

TEST_P(RgbReduction, averages_red_without_mixing_in_green_or_blue) // NOLINT
{
  const unsigned firstRedValue = 20;
  const unsigned secondRedValue = 60;
  const unsigned greenValue = 100;
  const unsigned blueValue = 180;
  const auto wholeTile = tileArea(source);
  const Scroom::Utils::Rectangle topHalf(0, 0, source->width, source->height / 2);
  fillChannel(source, format, wholeTile, RgbChannels::RED, secondRedValue);
  fillChannel(source, format, topHalf, RgbChannels::RED, firstRedValue);
  fillChannel(source, format, wholeTile, RgbChannels::GREEN, greenValue);
  fillChannel(source, format, wholeTile, RgbChannels::BLUE, blueValue);

  reduceLayer(target, readOnly(source), 0, 0);

  EXPECT_EQ((firstRedValue + secondRedValue) / 2, targetChannel(RgbChannels::RED));
  EXPECT_EQ(greenValue, targetChannel(RgbChannels::GREEN));
  EXPECT_EQ(blueValue, targetChannel(RgbChannels::BLUE));
}

class CmykReduction : public testing::TestWithParam<int>
{
protected:
  const SampleLayout sourceFormat{GetParam(), CmykChannels::COUNT};
  // One-, two-, and four-bit channels expand to bytes; sixteen-bit channels keep their precision.
  const SampleLayout targetFormat{GetParam() == 16 ? 16 : 8, CmykChannels::COUNT};
  Tile::Ptr source{emptyTile(reductionFactor, sourceFormat)};
  Tile::Ptr target{emptyTile(reductionFactor, targetFormat)};
  LayerReduction reduceLayer{createCmykReduction(sourceFormat.bitsPerSample)};

  [[nodiscard]] unsigned targetChannel(int channel) const { return sample(target, targetFormat, 0, 0, channel); }
};

INSTANTIATE_TEST_SUITE_P(SampleWidths, CmykReduction, testing::Values(1, 2, 4, 8, 16));

TEST_P(CmykReduction, maps_each_channel_average_to_its_output_range) // NOLINT
{
  const unsigned fullSource = (1U << sourceFormat.bitsPerSample) - 1;
  const unsigned fullOutput = (1U << targetFormat.bitsPerSample) - 1;
  const auto wholeTile = tileArea(source);
  const Scroom::Utils::Rectangle leftHalf(0, 0, source->width / 2, source->height);
  const Scroom::Utils::Rectangle topQuarter(0, 0, source->width, source->height / 4);
  fillChannel(source, sourceFormat, wholeTile, CmykChannels::CYAN, fullSource);
  fillChannel(source, sourceFormat, leftHalf, CmykChannels::MAGENTA, fullSource);
  fillChannel(source, sourceFormat, topQuarter, CmykChannels::YELLOW, fullSource);
  // Black stays zero: the four channels cover all, half, a quarter, and none of the block.

  reduceLayer(target, readOnly(source), 0, 0);

  EXPECT_EQ(fullOutput, targetChannel(CmykChannels::CYAN));
  EXPECT_EQ(fullOutput / 2, targetChannel(CmykChannels::MAGENTA));
  EXPECT_EQ(fullOutput / 4, targetChannel(CmykChannels::YELLOW));
  EXPECT_EQ(0, targetChannel(CmykChannels::BLACK));
}

class ColormapReductions : public testing::TestWithParam<int>
{
protected:
  const int bitsPerIndex = GetParam();
  const SampleLayout indexFormat{bitsPerIndex, 1};
  const SampleLayout pairFormat{bitsPerIndex, 2};
  Tile::Ptr indices{emptyTile(reductionFactor, indexFormat)};
  Tile::Ptr pairs{emptyTile(reductionFactor, pairFormat)};
  Tile::Ptr target{emptyTile(reductionFactor, pairFormat)};
  LayerReduction reduceIndices{createColormapReduction(bitsPerIndex)};
  LayerReduction reducePairs{createColormapPairReduction(bitsPerIndex)};

  [[nodiscard]] unsigned firstReducedIndex() const { return sample<uint16_t>(target, pairFormat, 0, 0, 0); }
  [[nodiscard]] unsigned secondReducedIndex() const { return sample<uint16_t>(target, pairFormat, 0, 0, 1); }
};

// Two-bit and eight-bit indices exercise four-bit pairs and sixteen-bit pairs.
INSTANTIATE_TEST_SUITE_P(IndexStorage, ColormapReductions, testing::Values(2, 8));

TEST_P(ColormapReductions, retains_the_two_indices_with_the_most_source_rows) // NOLINT
{
  const unsigned dominantIndex = 2;
  const unsigned runnerUpIndex = 1;
  const unsigned thirdIndex = 3;
  const int dominantRows = 5;
  const int runnerUpRows = 2;
  const int remainingRows = indices->height - dominantRows - runnerUpRows;
  fillChannel(indices, indexFormat, {0, 0, indices->width, dominantRows}, 0, dominantIndex);
  fillChannel(indices, indexFormat, {0, dominantRows, indices->width, runnerUpRows}, 0, runnerUpIndex);
  fillChannel(indices, indexFormat, {0, dominantRows + runnerUpRows, indices->width, remainingRows}, 0, thirdIndex);

  reduceIndices(target, readOnly(indices), 0, 0);

  // Five rows beat two rows, which beat the single remaining row.
  EXPECT_EQ(dominantIndex, firstReducedIndex());
  EXPECT_EQ(runnerUpIndex, secondReducedIndex());
}

TEST_P(ColormapReductions, counts_both_members_of_each_source_pair) // NOLINT
{
  const unsigned dominantIndex = 1;
  const unsigned runnerUpIndex = 2;
  const unsigned thirdIndex = 3;
  const int runnerUpRows = 5;
  const auto wholeTile = tileArea(pairs);
  // Every second index is dominant; the first index is runner-up in five rows and third in three.
  fillChannel<uint16_t>(pairs, pairFormat, wholeTile, 0, thirdIndex);
  fillChannel<uint16_t>(pairs, pairFormat, {0, 0, pairs->width, runnerUpRows}, 0, runnerUpIndex);
  fillChannel<uint16_t>(pairs, pairFormat, wholeTile, 1, dominantIndex);

  reducePairs(target, readOnly(pairs), 0, 0);

  EXPECT_EQ(dominantIndex, firstReducedIndex());
  EXPECT_EQ(runnerUpIndex, secondReducedIndex());
}

TEST_P(ColormapReductions, repeats_the_index_when_all_source_pairs_contain_one_colour) // NOLINT
{
  const unsigned onlyIndex = 2;
  const auto wholeTile = tileArea(pairs);
  fillChannel<uint16_t>(pairs, pairFormat, wholeTile, 0, onlyIndex);
  fillChannel<uint16_t>(pairs, pairFormat, wholeTile, 1, onlyIndex);

  reducePairs(target, readOnly(pairs), 0, 0);

  EXPECT_EQ(onlyIndex, firstReducedIndex());
  EXPECT_EQ(onlyIndex, secondReducedIndex());
}

class BinaryOrLayerReduction : public testing::Test
{
protected:
  // Eight reduced one-bit pixels fill a byte, so each source tile spans eight blocks per side.
  static constexpr int tileSize = 8 * reductionFactor;
  const SampleLayout format{1, 1};
  Tile::Ptr source{emptyTile(tileSize, format)};
  Tile::Ptr target{emptyTile(tileSize, format)};
  LayerReduction reduceLayer{createBinaryOrReduction()};
};

TEST_F(BinaryOrLayerReduction, a_single_set_bit_marks_its_block_and_leaves_neighbouring_blocks_clear) // NOLINT
{
  const int blockColumn = 2;
  const int blockRow = 3;
  const int sourceColumn = blockColumn * reductionFactor + reductionFactor / 2;
  const int sourceRow = blockRow * reductionFactor + reductionFactor / 2;
  const unsigned setBit = 1;
  setSample(source, format, sourceColumn, sourceRow, 0, setBit);

  reduceLayer(target, readOnly(source), 0, 0);

  EXPECT_EQ(setBit, sample(target, format, blockColumn, blockRow, 0));
  EXPECT_EQ(0, sample(target, format, blockColumn - 1, blockRow, 0));
  EXPECT_EQ(0, sample(target, format, blockColumn + 1, blockRow, 0));
  EXPECT_EQ(0, sample(target, format, blockColumn, blockRow - 1, 0));
  EXPECT_EQ(0, sample(target, format, blockColumn, blockRow + 1, 0));
}
