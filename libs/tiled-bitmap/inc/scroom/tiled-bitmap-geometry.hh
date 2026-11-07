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

  Rectangle<int> reducedBounds(Rectangle<int> imageAreaInLayerCoordinates);
  std::vector<Point<int>> pyramidSizes(Point<int> bottomSize, int minimumReducedDimension);
  int tileCount(int size, int tileSize);
  Point<int> layerCoordinatesFromTileIndex(Point<int> tileIndex, int tileSize);
  Rectangle<int> tileAreaInLayerCoordinates(Point<int> tileIndex, int tileSize);

  Rectangle<int> tileRangeForRedraw(Rectangle<double> layerArea, int tileSize);
  Rectangle<int> enclosingTileRange(Rectangle<int> layerArea, int tileSize);

  // The parent uses floor division; offsets are in [0, 8), also for negative indices.
  struct ReductionMapping
  {
    Point<int> targetIndex;
    Point<int> sourceOffset;
  };

  ReductionMapping reductionMapping(Point<int> sourceTileIndex);

  struct LayerSelection
  {
    std::size_t index{};
    int zoom{};
    // Requested presentation area expressed in the selected layer.
    Rectangle<double> presentationAreaInLayerCoordinates;
  };

  // layerCount must be nonzero; presentation coordinates match the bottom layer.
  LayerSelection layerSelection(Rectangle<double> presentationArea, int zoom, std::size_t layerCount);
  // Clip trailing edges while retaining the requested view origin. Leading
  // zero-origin image edges are handled by tileRangeForRedraw().
  Rectangle<double> clippedArea(Rectangle<double> requestedArea, Rectangle<int> imageArea);

  template <typename T>
  Rectangle<T> intersectionInTileCoordinates(Rectangle<T> areaInLayerCoordinates, Rectangle<int> tileAreaInLayerCoordinates)
  {
    const Rectangle<T> tileBoundsInLayerCoordinates(tileAreaInLayerCoordinates);
    if(!tileBoundsInLayerCoordinates.intersects(areaInLayerCoordinates))
    {
      return {};
    }
    return tileBoundsInLayerCoordinates.intersection(areaInLayerCoordinates) - tileBoundsInLayerCoordinates.getTopLeft();
  }

  struct TileDrawAreas
  {
    Rectangle<double> tileArea;
    Rectangle<double> viewArea;
  };

  TileDrawAreas tileDrawAreas(Rectangle<double> requestedArea, Rectangle<int> tileBounds, double pixelSize);

  struct RowLayout
  {
    int startLine;
    int lineCount;
  };

  // Rows at or past imageHeight contain zero lines. tileRow must be nonnegative.
  RowLayout rowLayout(int tileRow, int imageHeight, int tileSize);
} // namespace Scroom::TiledBitmap::Geometry
