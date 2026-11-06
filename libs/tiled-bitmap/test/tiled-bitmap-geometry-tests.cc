/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <limits>

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
  const auto range = Geometry::tileRangeForSampling({tileSize, 2 * tileSize, tileSize, tileSize}, tileSize);
  EXPECT_EQ(1, range.getLeft());
  EXPECT_EQ(2, range.getRight());
  EXPECT_EQ(2, range.getTop());
  EXPECT_EQ(3, range.getBottom());

  const auto singlePixel = Geometry::tileRangeForSampling({tileSize - 1, 0, 1, 1}, tileSize);
  EXPECT_EQ(0, singlePixel.getLeft());
  EXPECT_EQ(1, singlePixel.getRight());
  EXPECT_EQ(0, singlePixel.getTop());
  EXPECT_EQ(1, singlePixel.getBottom());
}

TEST_P(TileGeometry, sampling_range_covers_request_with_tiles_at_exact_grid_positions) // NOLINT
{
  const Rectangle requestedAreaInLayerCoordinates(tileSize - 2, 2 * tileSize - 1, 5, 4);
  const auto range = Geometry::tileRangeForSampling(requestedAreaInLayerCoordinates, tileSize);

  ASSERT_LT(range.getLeft(), range.getRight());
  ASSERT_LT(range.getTop(), range.getBottom());
  const auto coveredArea = range * tileSize;
  EXPECT_TRUE(coveredArea.contains(requestedAreaInLayerCoordinates));

  for(const auto [x, y]: range)
  {
    SCOPED_TRACE(testing::Message() << "tile " << x << ", " << y);
    const auto tileAreaInLayerCoordinates = Geometry::tileAreaInLayerCoordinates({x, y}, tileSize);
    EXPECT_EQ(Rectangle(x * tileSize, y * tileSize, tileSize, tileSize), tileAreaInLayerCoordinates);
    EXPECT_TRUE(tileAreaInLayerCoordinates.intersects(requestedAreaInLayerCoordinates));
  }
}

TEST_P(TileGeometry, sampling_uses_tile_local_coordinates) // NOLINT
{
  const Rectangle requestedAreaInLayerCoordinates(tileSize - 2, 2 * tileSize - 1, 5, 4);
  EXPECT_EQ(
    Rectangle(0, 0, 3, 3),
    Geometry::intersectionInTileCoordinates(
      requestedAreaInLayerCoordinates, Geometry::tileAreaInLayerCoordinates({1, 2}, tileSize)
    )
  );
  EXPECT_EQ(
    Rectangle(tileSize - 2, tileSize - 1, 2, 1),
    Geometry::intersectionInTileCoordinates(
      requestedAreaInLayerCoordinates, Geometry::tileAreaInLayerCoordinates({0, 1}, tileSize)
    )
  );
}

TEST_P(TileGeometry, disjoint_or_touching_requests_have_no_tile_pixels) // NOLINT
{
  const auto tileAreaInLayerCoordinates = Geometry::tileAreaInLayerCoordinates({1, 2}, tileSize);
  EXPECT_TRUE(Geometry::intersectionInTileCoordinates(Rectangle(0, 0, 1, 1), tileAreaInLayerCoordinates).isEmpty());
  EXPECT_TRUE(
    Geometry::intersectionInTileCoordinates(Rectangle(tileSize - 1, 2 * tileSize, 1, 1), tileAreaInLayerCoordinates).isEmpty()
  );
  EXPECT_TRUE(
    Geometry::intersectionInTileCoordinates(Rectangle(tileSize, 2 * tileSize, 0, 1), tileAreaInLayerCoordinates).isEmpty()
  );
}

TEST_P(TileGeometry, redraw_selects_tiles_for_an_aligned_rectangle) // NOLINT
{
  const auto range = Geometry::tileRangeForRedraw(Rectangle<double>(tileSize, 2 * tileSize, tileSize, tileSize), tileSize);
  EXPECT_EQ(1, range.getLeft());
  EXPECT_EQ(2, range.getRight());
  EXPECT_EQ(2, range.getTop());
  EXPECT_EQ(3, range.getBottom());
}

TEST_P(TileGeometry, redraw_keeps_tiles_on_both_sides_of_a_request) // NOLINT
{
  const Rectangle<double> requestedAreaInLayerCoordinates(tileSize - 2, 2 * tileSize - 1, 5, 4);
  const auto range = Geometry::tileRangeForRedraw(requestedAreaInLayerCoordinates, tileSize);
  EXPECT_EQ(0, range.getLeft());
  EXPECT_EQ(2, range.getRight());
  EXPECT_EQ(1, range.getTop());
  EXPECT_EQ(3, range.getBottom());
}

TEST_P(TileGeometry, requests_wholly_left_or_above_do_not_visit_tiles) // NOLINT
{
  const auto left = Geometry::tileRangeForRedraw(Rectangle<double>(-3 * tileSize, 0, tileSize, tileSize), tileSize);
  EXPECT_TRUE(left.isEmpty());
  const auto above = Geometry::tileRangeForRedraw(Rectangle<double>(0, -3 * tileSize, tileSize, tileSize), tileSize);
  EXPECT_TRUE(above.isEmpty());
}

TEST_P(TileGeometry, empty_requests_do_not_select_tiles) // NOLINT
{
  for(const auto& area: {Rectangle(tileSize - 1, 3, 0, 4), Rectangle(2, tileSize - 1, 5, 0)})
  {
    EXPECT_TRUE(Geometry::tileRangeForSampling(area, tileSize).isEmpty());
    EXPECT_TRUE(Geometry::tileRangeForRedraw(Rectangle<double>(area), tileSize).isEmpty());
  }
}

TEST_P(TileGeometry, sampling_rounds_negative_bounds_outward_on_the_same_grid) // NOLINT
{
  const Rectangle requestedAreaInLayerCoordinates(-tileSize - 2, -1, 3, tileSize + 2);
  const auto range = Geometry::tileRangeForSampling(requestedAreaInLayerCoordinates, tileSize);
  EXPECT_EQ(Rectangle(-2, -1, 2, 3), range);
  EXPECT_EQ(Rectangle(-2 * tileSize, -tileSize, tileSize, tileSize), Geometry::tileAreaInLayerCoordinates({-2, -1}, tileSize));
}

TEST_P(TileGeometry, redraw_clips_negative_tile_indices_without_moving_positive_bounds) // NOLINT
{
  const auto range =
    Geometry::tileRangeForRedraw(Rectangle<double>(-tileSize, -2 * tileSize, 2 * tileSize, 3 * tileSize), tileSize);
  EXPECT_EQ(Rectangle(0, 0, 1, 1), range);
}

TEST_P(TileGeometry, integer_screen_rectangles_map_to_the_same_source_pixels_at_power_of_two_zooms) // NOLINT
{
  const Rectangle<double> viewArea(0, 0, 4, 3);
  const Point<double> viewOriginInLayerCoordinates(tileSize - 1, 2 * tileSize - 1);
  const auto tileAreaInLayerCoordinates = Geometry::tileAreaInLayerCoordinates({1, 2}, tileSize);
  for(const double layerPixelSize: {0.5, 1.0, 2.0})
  {
    const auto requestedAreaInLayerCoordinates = viewArea / layerPixelSize + viewOriginInLayerCoordinates;
    const auto drawAreas = Geometry::tileDrawAreas(requestedAreaInLayerCoordinates, tileAreaInLayerCoordinates, layerPixelSize);
    EXPECT_EQ(Rectangle<double>(0, 0, 4 / layerPixelSize - 1, 3 / layerPixelSize - 1), drawAreas.tileArea);
    EXPECT_EQ(Rectangle<double>(layerPixelSize, layerPixelSize, 4 - layerPixelSize, 3 - layerPixelSize), drawAreas.viewArea);
    EXPECT_EQ(
      drawAreas.tileArea + Point<double>(tileAreaInLayerCoordinates.getTopLeft()),
      drawAreas.viewArea / layerPixelSize + requestedAreaInLayerCoordinates.getTopLeft()
    );
  }
}

TEST_P(TileGeometry, image_clipping_limits_the_last_tiles_without_shifting_the_view_origin) // NOLINT
{
  const Rectangle imageAreaInLayerCoordinates(0, 0, tileSize + 3, 2 * tileSize + 1);
  const Rectangle<double> requestedAreaInLayerCoordinates(tileSize - 2, 2 * tileSize - 1, 7, 4);
  const auto clippedAreaInLayerCoordinates = Geometry::clippedArea(requestedAreaInLayerCoordinates, imageAreaInLayerCoordinates);
  EXPECT_EQ(Rectangle<double>(tileSize - 2, 2 * tileSize - 1, 5, 2), clippedAreaInLayerCoordinates);
  const auto drawAreas =
    Geometry::tileDrawAreas(clippedAreaInLayerCoordinates, Geometry::tileAreaInLayerCoordinates({1, 2}, tileSize), 1.0);
  EXPECT_EQ(Rectangle<double>(0, 0, 3, 1), drawAreas.tileArea);
  EXPECT_EQ(Rectangle<double>(2, 1, 3, 1), drawAreas.viewArea);
}

TEST_P(TileGeometry, reduction_places_source_blocks_in_the_correct_parent_subregion) // NOLINT
{
  const int reductionFactor = 8;
  const Point targetTileIndex(2, 1);
  const Rectangle sourceTilesInTarget(0, 0, reductionFactor, reductionFactor);
  for(const auto sourceOffset: sourceTilesInTarget)
  {
    const auto sourceTileIndex = targetTileIndex * reductionFactor + sourceOffset;
    const auto mapping = Geometry::reductionMapping(sourceTileIndex);
    EXPECT_EQ(targetTileIndex, mapping.targetIndex);
    EXPECT_EQ(sourceOffset, mapping.sourceOffset);
    const auto sourceTileOriginInTargetLayerCoordinates = Geometry::layerCoordinatesFromTileIndex(mapping.targetIndex, tileSize)
                                                          + mapping.sourceOffset * (tileSize / reductionFactor);
    EXPECT_EQ(
      Geometry::layerCoordinatesFromTileIndex(sourceTileIndex, tileSize) / reductionFactor,
      sourceTileOriginInTargetLayerCoordinates
    );
  }
}

TEST_P(TileGeometry, row_layout_covers_the_image_once_and_stops_at_its_bottom_edge) // NOLINT
{
  for(const int imageHeight: {1, tileSize - 1, tileSize, tileSize + 1, 2 * tileSize + 3})
  {
    int nextLine = 0;
    for(int row = 0; row < Geometry::tileCount(imageHeight, tileSize); row++)
    {
      const auto layout = Geometry::rowLayout(row, imageHeight, tileSize);
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
  const Rectangle<double> presentationArea(64, 192, 320, 448);
  const auto selection = Geometry::layerSelection(presentationArea, -7, 4);
  EXPECT_EQ(2U, selection.index);
  EXPECT_EQ(-1, selection.zoom);
  EXPECT_EQ(Rectangle<double>(1, 3, 5, 7), selection.presentationAreaInLayerCoordinates);
}

TEST(LayerGeometry, layer_selection_preserves_fractional_offsets) // NOLINT
{
  const auto selection = Geometry::layerSelection({3, 5, 16, 24}, -3, 2);
  EXPECT_EQ(1U, selection.index);
  EXPECT_EQ(0, selection.zoom);
  EXPECT_EQ(Rectangle<double>(0.375, 0.625, 2, 3), selection.presentationAreaInLayerCoordinates);
}

TEST(LayerGeometry, layer_selection_retains_residual_zoom_when_fewer_levels_are_available) // NOLINT
{
  const Rectangle<double> presentationArea(64, 192, 320, 448);
  const auto bottomOnly = Geometry::layerSelection(presentationArea, -7, 1);
  EXPECT_EQ(0U, bottomOnly.index);
  EXPECT_EQ(-7, bottomOnly.zoom);
  EXPECT_EQ(presentationArea, bottomOnly.presentationAreaInLayerCoordinates);

  const auto selection = Geometry::layerSelection(presentationArea, -7, 2);
  EXPECT_EQ(1U, selection.index);
  EXPECT_EQ(-4, selection.zoom);
  EXPECT_EQ(Rectangle<double>(8, 24, 40, 56), selection.presentationAreaInLayerCoordinates);
}

TEST(LayerGeometry, positive_and_small_negative_zooms_use_the_original_coordinates) // NOLINT
{
  const Rectangle<double> presentationArea(3, 5, 17, 29);
  for(const int zoom: {-2, -1, 0, 3})
  {
    const auto selection = Geometry::layerSelection(presentationArea, zoom, 4);
    EXPECT_EQ(0U, selection.index);
    EXPECT_EQ(zoom, selection.zoom);
    EXPECT_EQ(presentationArea, selection.presentationAreaInLayerCoordinates);
  }
}

TEST(LayerGeometry, negative_source_tiles_map_to_nonnegative_offsets_in_their_parent) // NOLINT
{
  const auto mapping = Geometry::reductionMapping({-1, -9});
  EXPECT_EQ(Point(-1, -2), mapping.targetIndex);
  EXPECT_EQ(Point(7, 7), mapping.sourceOffset);
  const auto aligned = Geometry::reductionMapping({-8, -16});
  EXPECT_EQ(Point(-1, -2), aligned.targetIndex);
  EXPECT_EQ(Point(0, 0), aligned.sourceOffset);
}

TEST(LayerGeometry, reduced_bounds_round_all_four_edges_outward_around_the_shared_origin) // NOLINT
{
  EXPECT_EQ(Rectangle(0, 0, 3, 4), Geometry::reducedBounds({3, 5, 16, 24}));
  EXPECT_EQ(Rectangle(-2, 0, 3, 3), Geometry::reducedBounds({-9, 3, 17, 14}));
  EXPECT_EQ(Rectangle(-2, -3, 1, 2), Geometry::reducedBounds({-16, -24, 8, 16}));
}

TEST(LayerGeometry, reduced_bounds_preserve_empty_images) // NOLINT
{
  EXPECT_TRUE(Geometry::reducedBounds({3, 5, 0, 24}).isEmpty());
  EXPECT_TRUE(Geometry::reducedBounds({3, 5, 16, 0}).isEmpty());
}

TEST(LayerGeometry, repeated_reduction_keeps_the_same_coordinate_origin) // NOLINT
{
  const auto first = Geometry::reducedBounds({-9, 5, 65, 129});
  EXPECT_EQ(Rectangle(-2, 0, 9, 17), first);
  EXPECT_EQ(Rectangle(-1, 0, 2, 3), Geometry::reducedBounds(first));
}

TEST_P(TileGeometry, rows_at_or_beyond_the_image_bottom_are_empty) // NOLINT
{
  EXPECT_EQ(0, Geometry::rowLayout(0, 0, tileSize).lineCount);
  const auto row = Geometry::rowLayout(3, 2 * tileSize, tileSize);
  EXPECT_EQ(3 * tileSize, row.startLine);
  EXPECT_EQ(0, row.lineCount);
  EXPECT_EQ(0, Geometry::tileCount(0, tileSize));
}

TEST(LayerGeometry, disjoint_extreme_coordinates_do_not_overflow_during_intersection) // NOLINT
{
  const Rectangle tileAreaInLayerCoordinates(std::numeric_limits<int>::min(), 0, 1, 1);
  const Rectangle requestedAreaInLayerCoordinates(std::numeric_limits<int>::max() - 1, 0, 1, 1);
  EXPECT_TRUE(Geometry::intersectionInTileCoordinates(requestedAreaInLayerCoordinates, tileAreaInLayerCoordinates).isEmpty());
  const auto drawAreas =
    Geometry::tileDrawAreas(Rectangle<double>(requestedAreaInLayerCoordinates), tileAreaInLayerCoordinates, 1.0);
  EXPECT_TRUE(drawAreas.tileArea.isEmpty());
  EXPECT_TRUE(drawAreas.viewArea.isEmpty());
}

TEST(LayerGeometry, outward_rounding_near_integer_limits_keeps_the_last_pixel) // NOLINT
{
  const int maximum = std::numeric_limits<int>::max();
  EXPECT_EQ(maximum, Geometry::tileCount(maximum, 1));
  EXPECT_EQ(maximum / 8 + 1, Geometry::tileCount(maximum, 8));
  EXPECT_EQ(Rectangle(maximum - 1, 0, 1, 1), Geometry::tileRangeForSampling({maximum - 1, 0, 1, 1}, 1));
  EXPECT_EQ(Rectangle(0, 0, maximum / 8 + 1, 1), Geometry::reducedBounds({0, 0, maximum, 1}));

  const auto mapping = Geometry::reductionMapping({std::numeric_limits<int>::min(), maximum});
  EXPECT_EQ(Point(std::numeric_limits<int>::min() / 8, maximum / 8), mapping.targetIndex);
  EXPECT_EQ(Point(0, 7), mapping.sourceOffset);
}
