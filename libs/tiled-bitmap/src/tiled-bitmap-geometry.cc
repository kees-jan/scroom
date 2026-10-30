/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <algorithm>

#include <scroom/assertions.hh>
#include <scroom/tiled-bitmap-geometry.hh>

namespace Scroom::TiledBitmap::Geometry
{
  Rectangle<int> computeReducedBounds(Rectangle<int> imageBounds)
  {
    if(imageBounds.isEmpty())
    {
      return {};
    }
    return roundOutward(Rectangle<double>(imageBounds) / 8).to<int>();
  }

  std::vector<Point<int>> computePyramidSizes(Point<int> bottomSize, int minimumReducedDimension)
  {
    require(bottomSize.x >= 0 && bottomSize.y >= 0);
    require(minimumReducedDimension > 0);
    std::vector<Point<int>> sizes{bottomSize};
    auto bounds = computeReducedBounds(Rectangle<int>(0, 0, bottomSize.x, bottomSize.y));
    while(std::max(bounds.getWidth(), bounds.getHeight()) > minimumReducedDimension)
    {
      sizes.emplace_back(bounds.getWidth(), bounds.getHeight());
      bounds = computeReducedBounds(bounds);
    }
    return sizes;
  }

  int computeTileCount(int size, int tileSize)
  {
    require(size >= 0);
    require(tileSize > 0);
    return ceiled_divide_by(size, tileSize);
  }

  Point<int> computeTileOrigin(Point<int> tileIndex, int tileSize)
  {
    require(tileSize > 0);
    return tileIndex * tileSize;
  }

  Rectangle<int> computeTileRectangle(Point<int> tileIndex, int tileSize)
  {
    require(tileSize > 0);
    return (Rectangle<int>(0, 0, 1, 1) + tileIndex) * tileSize;
  }

  Rectangle<int> computeTileRangeForRedraw(Rectangle<double> layerArea, int tileSize)
  {
    require(tileSize > 0);
    if(layerArea.isEmpty())
    {
      return {};
    }
    const int left = layerArea.getLeft();
    const int top = layerArea.getTop();
    const int right = layerArea.getRight();
    const int bottom = layerArea.getBottom();
    const int xBegin = std::max(0, floored_divide_by(left, tileSize));
    const int yBegin = std::max(0, floored_divide_by(top, tileSize));
    const int xEnd = ceiled_divide_by(right, tileSize);
    const int yEnd = ceiled_divide_by(bottom, tileSize);
    return {xBegin, yBegin, std::max(0, xEnd - xBegin), std::max(0, yEnd - yBegin)};
  }

  Rectangle<int> computeTileRangeForSampling(Rectangle<int> layerArea, int tileSize)
  {
    require(tileSize > 0);
    if(layerArea.isEmpty())
    {
      return {};
    }
    return roundOutward(Rectangle<double>(layerArea) / tileSize).to<int>();
  }

  ReductionMapping computeReductionMapping(Point<int> sourceTile)
  {
    const Point<int> targetTile(floored_divide_by(sourceTile.x, 8), floored_divide_by(sourceTile.y, 8));
    return {targetTile, sourceTile - targetTile * 8};
  }

  LayerSelection computeLayerSelection(Rectangle<double> area, int zoom, std::size_t layerCount)
  {
    require(layerCount > 0);
    std::size_t layer = 0;
    while(zoom <= -3 && layer < layerCount - 1)
    {
      layer++;
      zoom += 3;
      area /= 8;
    }
    return {layer, zoom, area};
  }

  Rectangle<double> computeClippedDrawArea(Rectangle<double> requestedArea, Rectangle<int> imageBounds)
  {
    const auto validArea = requestedArea.intersection(imageBounds);
    return requestedArea.above(validArea.getBottom()).leftOf(validArea.getRight());
  }

  TileDrawAreas computeTileDrawAreas(Rectangle<double> clippedLayerArea, Rectangle<int> tileBounds, double pixelSize)
  {
    require(pixelSize > 0);
    const auto source = computeTileLocalIntersection(clippedLayerArea, tileBounds);
    if(source.isEmpty())
    {
      return {};
    }
    const auto visibleArea = source + Point<double>(tileBounds.getTopLeft());
    return {source, (visibleArea - clippedLayerArea.getTopLeft()) * pixelSize};
  }

  LoadRow computeRowLayout(int tileRow, int imageHeight, int tileSize)
  {
    require(tileRow >= 0 && imageHeight >= 0);
    require(tileSize > 0);
    const int startLine = tileRow * tileSize;
    return {startLine, std::max(0, std::min(tileSize, imageHeight - startLine))};
  }
} // namespace Scroom::TiledBitmap::Geometry
