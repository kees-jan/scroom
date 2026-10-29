/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <algorithm>

#include <scroom/tiled-bitmap-geometry.hh>

namespace Scroom::TiledBitmap::Geometry
{
  std::vector<Point<int>> pyramidSizes(Point<int> bottomSize, int minimumReducedDimension)
  {
    std::vector<Point<int>> sizes{bottomSize};
    int width = (bottomSize.x + 7) / 8;
    int height = (bottomSize.y + 7) / 8;
    while(std::max(width, height) > minimumReducedDimension)
    {
      sizes.emplace_back(width, height);
      width = (width + 7) / 8;
      height = (height + 7) / 8;
    }
    return sizes;
  }

  int tileCount(int size, int tileSize) { return (size + tileSize - 1) / tileSize; }

  Point<int> tileOrigin(Point<int> tileIndex, int tileSize) { return tileIndex * tileSize; }

  Rectangle<int> tileRectangle(Point<int> tileIndex, int tileSize) { return (Rectangle<int>(0, 0, 1, 1) + tileIndex) * tileSize; }

  TileRange redrawTileRange(Rectangle<double> layerArea, int tileSize)
  {
    const int left = layerArea.getLeft();
    const int top = layerArea.getTop();
    const int right = layerArea.getRight();
    const int bottom = layerArea.getBottom();
    return {
      std::max(0, left / tileSize),
      (right + tileSize - 1) / tileSize,
      std::max(0, top / tileSize),
      (bottom + tileSize - 1) / tileSize
    };
  }

  TileRange samplingTileRange(Rectangle<int> layerArea, int tileSize)
  {
    return {
      layerArea.getLeft() / tileSize,
      (layerArea.getRight() - 1) / tileSize + 1,
      layerArea.getTop() / tileSize,
      (layerArea.getBottom() - 1) / tileSize + 1
    };
  }

  ReductionMapping reductionMapping(Point<int> sourceTile)
  {
    return {{sourceTile.x / 8, sourceTile.y / 8}, {sourceTile.x % 8, sourceTile.y % 8}};
  }

  LayerSelection selectLayer(Rectangle<double> area, int zoom, std::size_t layerCount)
  {
    std::size_t layer = 0;
    while(zoom <= -3 && layer < layerCount - 1)
    {
      layer++;
      zoom += 3;
      area /= 8;
    }
    return {layer, zoom, area};
  }

  Rectangle<double> clippedDrawArea(Rectangle<double> requestedArea, Rectangle<int> imageBounds)
  {
    const auto validArea = requestedArea.intersection(imageBounds);
    return requestedArea.above(validArea.getBottom()).leftOf(validArea.getRight());
  }

  TileDrawAreas tileDrawAreas(Rectangle<double> clippedLayerArea, Rectangle<int> tileBounds, double pixelSize)
  {
    const auto visibleArea = tileBounds.intersection(clippedLayerArea);
    return {tileLocalIntersection(clippedLayerArea, tileBounds), (visibleArea - clippedLayerArea.getTopLeft()) * pixelSize};
  }

  LoadRow computeRowLayout(int tileRow, int imageHeight, int tileSize)
  {
    return {tileRow * tileSize, std::min(tileSize, imageHeight - tileRow * tileSize)};
  }
} // namespace Scroom::TiledBitmap::Geometry
