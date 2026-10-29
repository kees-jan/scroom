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

  // These functions preserve the existing zero-origin arithmetic, including
  // truncation in redrawTileRange(). Relocation/rounding contracts follow later.
  std::vector<Point<int>> pyramidSizes(Point<int> bottomSize, int minimumReducedDimension);
  int tileCount(int size, int tileSize);
  Point<int> tileOrigin(Point<int> tileIndex, int tileSize);
  Rectangle<int> tileRectangle(Point<int> tileIndex, int tileSize);

  // End indices are exclusive. Keep redraw and sampling rounding separate.
  struct TileRange
  {
    int xBegin;
    int xEnd;
    int yBegin;
    int yEnd;
  };

  TileRange redrawTileRange(Rectangle<double> layerArea, int tileSize);
  TileRange samplingTileRange(Rectangle<int> layerArea, int tileSize);

  struct ReductionMapping
  {
    Point<int> targetTile;
    Point<int> offset;
  };

  ReductionMapping reductionMapping(Point<int> sourceTile);

  struct LayerSelection
  {
    std::size_t layer{};
    int zoom{};
    Rectangle<double> area;
  };

  // layerCount must be nonzero; area is in bottom-layer coordinates.
  LayerSelection selectLayer(Rectangle<double> area, int zoom, std::size_t layerCount);
  Rectangle<double> clippedDrawArea(Rectangle<double> requestedArea, Rectangle<int> imageBounds);

  template <typename T>
  Rectangle<T> tileLocalIntersection(Rectangle<T> layerArea, Rectangle<int> tileBounds)
  {
    return tileBounds.intersection(layerArea) - Point<T>(tileBounds.getTopLeft());
  }

  struct TileDrawAreas
  {
    Rectangle<double> source;
    Rectangle<double> destination;
  };

  TileDrawAreas tileDrawAreas(Rectangle<double> clippedLayerArea, Rectangle<int> tileBounds, double pixelSize);

  struct LoadRow
  {
    int startLine;
    int lineCount;
  };

  LoadRow computeRowLayout(int tileRow, int imageHeight, int tileSize);
} // namespace Scroom::TiledBitmap::Geometry
