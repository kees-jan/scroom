/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstdio>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include <scroom/memoryblobs.hh>
#include <scroom/stuff.hh>
#include <scroom/threadpool.hh>
#include <scroom/tile.hh>
#include <scroom/tiled-bitmap-geometry.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/tiledbitmaplayer.hh>
#include <scroom/viewinterface.hh>

#include "local.hh"

namespace Geometry = Scroom::TiledBitmap::Geometry;

Scroom::MemoryBlobs::PageProvider::Ptr createCompressedLayerPageProvider(Scroom::Utils::Point<int> imageSize, int bpp)
{
  const double tileCount = (static_cast<double>(imageSize.x) * imageSize.y) / (TILESIZE * TILESIZE);
  const double tileSize = (bpp / 8.0) * TILESIZE * TILESIZE;

  const double guessedTileSizeAfterCompression = tileSize / 100;
  const int pagesize = 4096;
  const int pagesPerBlock = std::max(int(ceil(guessedTileSizeAfterCompression / 10 / pagesize)), 1);

  const int blockSize = pagesPerBlock * pagesize;
  const int blockCount = std::max(int(ceil(tileCount / 10)), 64);

  Scroom::Logger logger;
  logger->debug("Creating a PageProvider providing {} blocks of {} bytes", blockCount, blockSize);
  return Scroom::MemoryBlobs::PageProvider::create(blockCount, blockSize);
}


////////////////////////////////////////////////////////////////////////
/// Layer

Layer::Layer(int depth, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider)
  : width(layerWidth)
  , height(layerHeight)
  , bitsPerPixel(bpp)
  , horTileCount(Geometry::tileCount(width, TILESIZE))
  , verTileCount(Geometry::tileCount(height, TILESIZE))
  , pageProvider(std::move(provider))
{
  tiles.resize(verTileCount);
  const Scroom::Utils::Rectangle tileRange(0, 0, horTileCount, verTileCount);
  for(const auto tileIndex: tileRange)
  {
    tiles[tileIndex.y].push_back(CompressedTile::create(depth, tileIndex.x, tileIndex.y, bpp, pageProvider));
  }

  outOfBounds = CompressedTile::create(depth, -1, -1, bpp, pageProvider, TSI_OUT_OF_BOUNDS);

  logger->debug("Layer {} ({} bpp), {}*{}, TileCount {}*{}", depth, bpp, width, height, horTileCount, verTileCount);
}

Layer::Ptr Layer::create(int depth, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider)
{
  return Ptr(new Layer(depth, layerWidth, layerHeight, bpp, std::move(provider)));
}

Layer::Ptr Layer::create(int layerWidth, int layerHeight, int bpp)
{
  Scroom::MemoryBlobs::PageProvider::Ptr const provider = createCompressedLayerPageProvider({layerWidth, layerHeight}, bpp);

  return create(0, layerWidth, layerHeight, bpp, provider);
}

TileStorage::Ptr Layer::getTile(Scroom::Utils::Point<int> tileIndex)
{
  const auto [i, j] = tileIndex;
  if(0 <= i && i < horTileCount && 0 <= j && j < verTileCount)
  {
    return tiles[j][i];
  }

  return outOfBounds;
}

LayerFactory createCompressedLayerFactory(Scroom::MemoryBlobs::PageProvider::Ptr provider)
{
  require(provider);
  return [provider = std::move(provider)](int level, Scroom::Utils::Rectangle<int> imageArea, int bpp)
  {
    require(imageArea.getTopLeft() == Scroom::Utils::Point(0, 0));
    return Layer::create(level, imageArea.getWidth(), imageArea.getHeight(), bpp, provider);
  };
}
