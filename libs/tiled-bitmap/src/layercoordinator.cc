/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "layercoordinator.hh"

#include <cstdio>
#include <utility>

#include <scroom/threadpool.hh>
#include <scroom/tiledbitmaplayer.hh>

#include "local.hh"

LayerCoordinator::Ptr LayerCoordinator::create(CompressedTile::Ptr targetTile, Scroom::TiledBitmap::LayerReduction reduceLayer)
{
  return LayerCoordinator::Ptr(new LayerCoordinator(std::move(targetTile), std::move(reduceLayer)));
}

LayerCoordinator::LayerCoordinator(CompressedTile::Ptr targetTile_, Scroom::TiledBitmap::LayerReduction reduceLayer_)
  : targetTile(std::move(targetTile_))
  , reduceLayer(std::move(reduceLayer_))

{
}

LayerCoordinator::~LayerCoordinator()
{
  registrations.clear();
  sourceTiles.clear();
}

void LayerCoordinator::addSourceTile(int x, int y, const CompressedTile::Ptr& tile)
{
  boost::unique_lock<boost::mutex> const lock(mut);

  sourceTiles[tile] = std::make_pair(x, y);
  registrations.emplace_back(tile->registerObserver(shared_from_this<LayerCoordinator>()));
  unfinishedSourceTiles++;
}

////////////////////////////////////////////////////////////////////////
/// TileInitialisationObserver

void LayerCoordinator::tileFinished(const CompressedTile::Ptr& tile)
{
  ConstTile::Ptr const tileData = tile->getConstTileAsync();
  require(tileData);

  CpuBound()->post(
    [me = shared_from_this<LayerCoordinator>(), tile, tileData] { me->reduceSourceTile(tile, tileData); }, REDUCE_PRIO
  );
}

////////////////////////////////////////////////////////////////////////
/// Helpers

void LayerCoordinator::reduceSourceTile(const CompressedTile::Ptr& tile, ConstTile::Ptr const& /*tileData*/)
{
  // If tileData contains a valid pointer, then fetching
  // sourcetiledata, below, will be instananeous. Otherwise, it will
  // need to unzip the compressed tile.
  //
  // Other than that side-effect, we have no use for tileData
  Scroom::Utils::Stuff const s = targetTile->initialize();
  const std::pair<int, int> location = sourceTiles[tile];
  const int x = location.first;
  const int y = location.second;

  if(!targetTileData)
  {
    targetTileData = targetTile->getTileSync();
  }
  ConstTile::Ptr const source = tile->getConstTileSync();

  reduceLayer(targetTileData, source, x, y);

  boost::unique_lock<boost::mutex> const lock(mut);
  unfinishedSourceTiles--;
  if(!unfinishedSourceTiles)
  {
    targetTile->reportFinished();
    targetTileData.reset();
  }
}
