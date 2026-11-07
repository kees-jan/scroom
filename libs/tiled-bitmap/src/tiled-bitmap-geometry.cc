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
  Rectangle<int> reducedBounds(Rectangle<int> imageAreaInLayerCoordinates)
  {
    if(imageAreaInLayerCoordinates.isEmpty())
    {
      return {};
    }
    return roundOutward(Rectangle<double>(imageAreaInLayerCoordinates) / 8).to<int>();
  }

  std::vector<Point<int>> pyramidSizes(Point<int> bottomSize, int minimumReducedDimension)
  {
    require(bottomSize.x >= 0 && bottomSize.y >= 0);
    require(minimumReducedDimension > 0);
    std::vector<Point<int>> sizes{bottomSize};
    auto imageAreaInLayerCoordinates = reducedBounds(Rectangle<int>(0, 0, bottomSize.x, bottomSize.y));
    while(std::max(imageAreaInLayerCoordinates.getWidth(), imageAreaInLayerCoordinates.getHeight()) > minimumReducedDimension)
    {
      sizes.emplace_back(imageAreaInLayerCoordinates.getWidth(), imageAreaInLayerCoordinates.getHeight());
      imageAreaInLayerCoordinates = reducedBounds(imageAreaInLayerCoordinates);
    }
    return sizes;
  }

  int tileCount(int size, int tileSize)
  {
    require(size >= 0);
    require(tileSize > 0);
    return ceiled_divide_by(size, tileSize);
  }

  Point<int> layerCoordinatesFromTileIndex(Point<int> tileIndex, int tileSize)
  {
    require(tileSize > 0);
    return tileIndex * tileSize;
  }

  Rectangle<int> tileAreaInLayerCoordinates(Point<int> tileIndex, int tileSize)
  {
    require(tileSize > 0);
    return (Rectangle<int>(0, 0, 1, 1) + tileIndex) * tileSize;
  }

  Rectangle<int> tileRangeForRedraw(Rectangle<double> layerArea, int tileSize)
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
    const int xBegin = floored_divide_by(left, tileSize);
    const int yBegin = floored_divide_by(top, tileSize);
    const int xEnd = ceiled_divide_by(right, tileSize);
    const int yEnd = ceiled_divide_by(bottom, tileSize);
    return Scroom::Utils::make_rect_from_start_end(xBegin, yBegin, xEnd, yEnd).rightOf(0).below(0);
  }

  Rectangle<int> enclosingTileRange(Rectangle<int> layerArea, int tileSize)
  {
    require(tileSize > 0);
    if(layerArea.isEmpty())
    {
      return {};
    }
    return roundOutward(Rectangle<double>(layerArea) / tileSize).to<int>();
  }

  ReductionMapping reductionMapping(Point<int> sourceTileIndex)
  {
    const Point<int> targetTileIndex(floored_divide_by(sourceTileIndex.x, 8), floored_divide_by(sourceTileIndex.y, 8));
    return {targetTileIndex, sourceTileIndex - targetTileIndex * 8};
  }

  LayerSelection layerSelection(Rectangle<double> presentationArea, int zoom, std::size_t layerCount)
  {
    require(layerCount > 0);
    std::size_t layerIndex = 0;
    auto presentationAreaInLayerCoordinates = presentationArea;
    int layerZoom = zoom;
    while(layerZoom <= -3 && layerIndex < layerCount - 1)
    {
      layerIndex++;
      layerZoom += 3;
      presentationAreaInLayerCoordinates /= 8;
    }
    return {layerIndex, layerZoom, presentationAreaInLayerCoordinates};
  }

  Rectangle<double> clippedArea(Rectangle<double> requestedArea, Rectangle<int> imageArea)
  {
    const auto validArea = requestedArea.intersection(imageArea);
    return requestedArea.above(validArea.getBottom()).leftOf(validArea.getRight());
  }

  TileDrawAreas tileDrawAreas(Rectangle<double> requestedArea, Rectangle<int> tileBounds, double pixelSize)
  {
    require(pixelSize > 0);
    const auto tileArea = intersectionInTileCoordinates(requestedArea, tileBounds);
    if(tileArea.isEmpty())
    {
      return {};
    }
    const auto visibleArea = tileArea + Point<double>(tileBounds.getTopLeft());
    const auto viewArea = (visibleArea - requestedArea.getTopLeft()) * pixelSize;
    return {tileArea, viewArea};
  }

  RowLayout rowLayout(int tileRow, int imageHeight, int tileSize)
  {
    require(tileRow >= 0 && imageHeight >= 0);
    require(tileSize > 0);
    const int startLine = tileRow * tileSize;
    return {startLine, std::max(0, std::min(tileSize, imageHeight - startLine))};
  }
} // namespace Scroom::TiledBitmap::Geometry
