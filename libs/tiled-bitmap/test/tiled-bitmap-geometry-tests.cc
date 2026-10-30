/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gtest/gtest.h>

#include <scroom/tiled-bitmap-geometry.hh>

namespace Geometry = Scroom::TiledBitmap::Geometry;
using Scroom::Utils::Point;
using Scroom::Utils::Rectangle;

class TileGeometry : public testing::TestWithParam<int>
{
protected:
  const int tileSize{GetParam()};
};

INSTANTIATE_TEST_SUITE_P(TileSizes, TileGeometry, testing::Values(8, 24, 64));

TEST_P(TileGeometry, allocated_tiles_cover_the_image_without_an_extra_tile) // NOLINT
{
  for(const int size: {1, tileSize - 1, tileSize, tileSize + 1, 2 * tileSize + 3})
  {
    SCOPED_TRACE(size);
    const int count = Geometry::tileCount(size, tileSize);
    EXPECT_GE(count * tileSize, size);
    EXPECT_LT((count - 1) * tileSize, size);
  }
}

TEST_P(TileGeometry, sampling_excludes_tiles_that_only_touch_the_requested_edge) // NOLINT
{
  const auto range = Geometry::samplingTileRange({tileSize, 2 * tileSize, tileSize, tileSize}, tileSize);
  EXPECT_EQ(1, range.xBegin);
  EXPECT_EQ(2, range.xEnd);
  EXPECT_EQ(2, range.yBegin);
  EXPECT_EQ(3, range.yEnd);

  const auto singlePixel = Geometry::samplingTileRange({tileSize - 1, 0, 1, 1}, tileSize);
  EXPECT_EQ(0, singlePixel.xBegin);
  EXPECT_EQ(1, singlePixel.xEnd);
  EXPECT_EQ(0, singlePixel.yBegin);
  EXPECT_EQ(1, singlePixel.yEnd);
}

TEST_P(TileGeometry, sampling_range_covers_request_with_tiles_at_exact_grid_positions) // NOLINT
{
  const Rectangle<int> requested(tileSize - 2, 2 * tileSize - 1, 5, 4);
  const auto range = Geometry::samplingTileRange(requested, tileSize);

  ASSERT_LT(range.xBegin, range.xEnd);
  ASSERT_LT(range.yBegin, range.yEnd);
  const Rectangle<int> coveredArea(
    range.xBegin * tileSize,
    range.yBegin * tileSize,
    (range.xEnd - range.xBegin) * tileSize,
    (range.yEnd - range.yBegin) * tileSize
  );
  EXPECT_TRUE(coveredArea.contains(requested));

  for(int x = range.xBegin; x < range.xEnd; x++)
  {
    for(int y = range.yBegin; y < range.yEnd; y++)
    {
      SCOPED_TRACE(testing::Message() << "tile " << x << ", " << y);
      const auto tile = Geometry::tileRectangle({x, y}, tileSize);
      EXPECT_EQ(Rectangle<int>(x * tileSize, y * tileSize, tileSize, tileSize), tile);
      EXPECT_TRUE(tile.intersects(requested));
    }
  }
}

TEST_P(TileGeometry, sampling_uses_tile_local_coordinates) // NOLINT
{
  const Rectangle<int> requested(tileSize - 2, 2 * tileSize - 1, 5, 4);
  EXPECT_EQ(Rectangle<int>(0, 0, 3, 3), Geometry::tileLocalIntersection(requested, Geometry::tileRectangle({1, 2}, tileSize)));
  EXPECT_EQ(
    Rectangle<int>(tileSize - 2, tileSize - 1, 2, 1),
    Geometry::tileLocalIntersection(requested, Geometry::tileRectangle({0, 1}, tileSize))
  );
}

TEST_P(TileGeometry, disjoint_or_touching_requests_have_no_tile_pixels) // NOLINT
{
  const auto tile = Geometry::tileRectangle({1, 2}, tileSize);
  EXPECT_TRUE(Geometry::tileLocalIntersection(Rectangle<int>(0, 0, 1, 1), tile).isEmpty());
  EXPECT_TRUE(Geometry::tileLocalIntersection(Rectangle<int>(tileSize - 1, 2 * tileSize, 1, 1), tile).isEmpty());
  EXPECT_TRUE(Geometry::tileLocalIntersection(Rectangle<int>(tileSize, 2 * tileSize, 0, 1), tile).isEmpty());
}

TEST_P(TileGeometry, redraw_selects_tiles_for_an_aligned_rectangle) // NOLINT
{
  const auto range = Geometry::redrawTileRange(Rectangle<double>(tileSize, 2 * tileSize, tileSize, tileSize), tileSize);
  EXPECT_EQ(1, range.xBegin);
  EXPECT_EQ(2, range.xEnd);
  EXPECT_EQ(2, range.yBegin);
  EXPECT_EQ(3, range.yEnd);
}

TEST_P(TileGeometry, redraw_keeps_tiles_on_both_sides_of_a_request) // NOLINT
{
  const Rectangle<double> requested(tileSize - 2, 2 * tileSize - 1, 5, 4);
  const auto range = Geometry::redrawTileRange(requested, tileSize);
  EXPECT_EQ(0, range.xBegin);
  EXPECT_EQ(2, range.xEnd);
  EXPECT_EQ(1, range.yBegin);
  EXPECT_EQ(3, range.yEnd);
}

TEST_P(TileGeometry, requests_wholly_left_or_above_do_not_visit_tiles) // NOLINT
{
  const auto left = Geometry::redrawTileRange(Rectangle<double>(-3 * tileSize, 0, tileSize, tileSize), tileSize);
  EXPECT_GE(left.xBegin, left.xEnd);
  const auto above = Geometry::redrawTileRange(Rectangle<double>(0, -3 * tileSize, tileSize, tileSize), tileSize);
  EXPECT_GE(above.yBegin, above.yEnd);
}

TEST_P(TileGeometry, integer_screen_rectangles_map_to_the_same_source_pixels_at_power_of_two_zooms) // NOLINT
{
  const Rectangle<double> screenArea(0, 0, 4, 3);
  const Point<double> offset(tileSize - 1, 2 * tileSize - 1);
  const auto tile = Geometry::tileRectangle({1, 2}, tileSize);
  for(const double pixelSize: {0.5, 1.0, 2.0})
  {
    const auto requested = screenArea / pixelSize + offset;
    const auto areas = Geometry::tileDrawAreas(requested, tile, pixelSize);
    EXPECT_EQ(Rectangle<double>(0, 0, 4 / pixelSize - 1, 3 / pixelSize - 1), areas.source);
    EXPECT_EQ(Rectangle<double>(pixelSize, pixelSize, 4 - pixelSize, 3 - pixelSize), areas.destination);
    EXPECT_EQ(areas.source + Point<double>(tile.getTopLeft()), areas.destination / pixelSize + requested.getTopLeft());
  }
}

TEST_P(TileGeometry, image_clipping_limits_the_last_tiles_without_shifting_the_view_origin) // NOLINT
{
  const Rectangle<int> image(0, 0, tileSize + 3, 2 * tileSize + 1);
  const Rectangle<double> requested(tileSize - 2, 2 * tileSize - 1, 7, 4);
  const auto clipped = Geometry::clippedDrawArea(requested, image);
  EXPECT_EQ(Rectangle<double>(tileSize - 2, 2 * tileSize - 1, 5, 2), clipped);
  const auto areas = Geometry::tileDrawAreas(clipped, Geometry::tileRectangle({1, 2}, tileSize), 1.0);
  EXPECT_EQ(Rectangle<double>(0, 0, 3, 1), areas.source);
  EXPECT_EQ(Rectangle<double>(2, 1, 3, 1), areas.destination);
}

TEST_P(TileGeometry, reduction_places_source_blocks_in_the_correct_parent_subregion) // NOLINT
{
  for(int x = 0; x < 8; x++)
  {
    for(int y = 0; y < 8; y++)
    {
      const Point<int> sourceTile(16 + x, 8 + y);
      const auto mapping = Geometry::reductionMapping(sourceTile);
      EXPECT_EQ(Point<int>(2, 1), mapping.targetTile);
      EXPECT_EQ(Point<int>(x, y), mapping.offset);
      const auto targetOrigin = Geometry::tileOrigin(mapping.targetTile, tileSize) + mapping.offset * (tileSize / 8);
      EXPECT_EQ(Geometry::tileOrigin(sourceTile, tileSize) / 8, targetOrigin);
    }
  }
}

TEST_P(TileGeometry, row_layout_covers_the_image_once_and_stops_at_its_bottom_edge) // NOLINT
{
  for(const int imageHeight: {1, tileSize - 1, tileSize, tileSize + 1, 2 * tileSize + 3})
  {
    int nextLine = 0;
    for(int row = 0; row < Geometry::tileCount(imageHeight, tileSize); row++)
    {
      const auto layout = Geometry::computeRowLayout(row, imageHeight, tileSize);
      EXPECT_EQ(nextLine, layout.startLine);
      EXPECT_GT(layout.lineCount, 0);
      EXPECT_LE(layout.lineCount, tileSize);
      nextLine += layout.lineCount;
      EXPECT_LE(nextLine, imageHeight);
    }
    EXPECT_EQ(imageHeight, nextLine);
  }
}

TEST(LayerGeometry, layer_selection_preserves_coordinates_across_multiple_reductions) // NOLINT
{
  const Rectangle<double> requested(64, 192, 320, 448);
  const auto selection = Geometry::selectLayer(requested, -7, 4);
  EXPECT_EQ(2U, selection.layer);
  EXPECT_EQ(-1, selection.zoom);
  EXPECT_EQ(Rectangle<double>(1, 3, 5, 7), selection.area);
}

TEST(LayerGeometry, layer_selection_preserves_fractional_offsets) // NOLINT
{
  const auto selection = Geometry::selectLayer({3, 5, 16, 24}, -3, 2);
  EXPECT_EQ(1U, selection.layer);
  EXPECT_EQ(0, selection.zoom);
  EXPECT_EQ(Rectangle<double>(0.375, 0.625, 2, 3), selection.area);
}

TEST(LayerGeometry, layer_selection_retains_residual_zoom_when_fewer_levels_are_available) // NOLINT
{
  const Rectangle<double> requested(64, 192, 320, 448);
  const auto bottomOnly = Geometry::selectLayer(requested, -7, 1);
  EXPECT_EQ(0U, bottomOnly.layer);
  EXPECT_EQ(-7, bottomOnly.zoom);
  EXPECT_EQ(requested, bottomOnly.area);

  const auto selection = Geometry::selectLayer(requested, -7, 2);
  EXPECT_EQ(1U, selection.layer);
  EXPECT_EQ(-4, selection.zoom);
  EXPECT_EQ(Rectangle<double>(8, 24, 40, 56), selection.area);
}

TEST(LayerGeometry, positive_and_small_negative_zooms_use_the_original_coordinates) // NOLINT
{
  const Rectangle<double> requested(3, 5, 17, 29);
  for(const int zoom: {-2, -1, 0, 3})
  {
    const auto selection = Geometry::selectLayer(requested, zoom, 4);
    EXPECT_EQ(0U, selection.layer);
    EXPECT_EQ(zoom, selection.zoom);
    EXPECT_EQ(requested, selection.area);
  }
}
