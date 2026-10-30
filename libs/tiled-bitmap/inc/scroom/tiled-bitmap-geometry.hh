/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstddef>
#include <vector>

#include <scroom/rectangle.hh>

namespace Scroom::TiledBitmap::Geometry
{
  using Scroom::Utils::Point;
  using Scroom::Utils::Rectangle;

  // Pixel and tile rectangles are half-open. Grids share coordinate origin zero;
  // an image offset is not a new grid origin. Reduction scales coordinates by 8.
  // Sizes are nonnegative and tile sizes are positive. Rectangle endpoints and
  // computed integer coordinates must fit in int; floating inputs must be finite.
  // Representability is the caller's responsibility, not checked by these helpers.
  // Reduced image bounds round outward around the shared origin.
  Rectangle<int> computeReducedBounds(Rectangle<int> imageBounds);
  // minimumReducedDimension must be positive so reduction terminates.
  std::vector<Point<int>> computePyramidSizes(Point<int> bottomSize, int minimumReducedDimension);
  int computeTileCount(int size, int tileSize);
  Point<int> computeTileOrigin(Point<int> tileIndex, int tileSize);
  Rectangle<int> computeTileRectangle(Point<int> tileIndex, int tileSize);

  // Half-open rectangles in tile-index coordinates. Redraw excludes negative
  // indices until relocated layers are supported; sampling does not clip them.
  // Redraw retains the current assumption of screen-derived bounds: fractional
  // pixel endpoints are truncated before selecting tiles.
  Rectangle<int> computeTileRangeForRedraw(Rectangle<double> layerArea, int tileSize);
  Rectangle<int> computeTileRangeForSampling(Rectangle<int> layerArea, int tileSize);

  // The parent uses floor division; offsets are in [0, 8), also for negative indices.
  struct ReductionMapping
  {
    Point<int> targetTile;
    Point<int> offset;
  };

  ReductionMapping computeReductionMapping(Point<int> sourceTile);

  struct LayerSelection
  {
    std::size_t layer{};
    int zoom{};
    Rectangle<double> area;
  };

  // layerCount must be nonzero; area is in bottom-layer coordinates.
  LayerSelection computeLayerSelection(Rectangle<double> area, int zoom, std::size_t layerCount);
  // Clip trailing edges while retaining the requested view origin. Leading
  // zero-origin image edges are handled by computeTileRangeForRedraw().
  Rectangle<double> computeClippedDrawArea(Rectangle<double> requestedArea, Rectangle<int> imageBounds);

  template <typename T>
  Rectangle<T> computeTileLocalIntersection(Rectangle<T> layerArea, Rectangle<int> tileBounds)
  {
    const Rectangle<T> bounds(tileBounds);
    if(!bounds.intersects(layerArea))
    {
      return {};
    }
    return bounds.intersection(layerArea) - bounds.getTopLeft();
  }

  struct TileDrawAreas
  {
    Rectangle<double> source;
    Rectangle<double> destination;
  };

  TileDrawAreas computeTileDrawAreas(Rectangle<double> clippedLayerArea, Rectangle<int> tileBounds, double pixelSize);

  struct LoadRow
  {
    int startLine;
    int lineCount;
  };

  // Rows at or past imageHeight contain zero lines. tileRow must be nonnegative.
  LoadRow computeRowLayout(int tileRow, int imageHeight, int tileSize);
} // namespace Scroom::TiledBitmap::Geometry
