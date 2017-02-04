/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "layercoordinator.hh"

#include <scroom/threadpool.hh>

#include "local.hh"

LayerCoordinator::Ptr LayerCoordinator::create(TileInternal::Ptr targetTile, LayerOperations::Ptr lo)
{
  return LayerCoordinator::Ptr(new LayerCoordinator(targetTile, lo));
}

LayerCoordinator::LayerCoordinator(TileInternal::Ptr targetTile,
                                   LayerOperations::Ptr lo)
  : targetTile(targetTile), lo(lo), unfinishedSourceTiles(0)
{
}

LayerCoordinator::~LayerCoordinator()
{
  registrations.clear();
  sourceTiles.clear();
}

void LayerCoordinator::addSourceTile(int x, int y, TileInternal::Ptr tile)
{
  boost::unique_lock<boost::mutex> lock(mut);

  sourceTiles[tile] = std::make_pair(x,y);
  registrations.push_back(tile->registerObserver(shared_from_this<LayerCoordinator>()));
  unfinishedSourceTiles++;
}

////////////////////////////////////////////////////////////////////////
/// TileInitialisationObserver

void LayerCoordinator::tileFinished(TileInternal::Ptr tile)
{
  CpuBound()->schedule(boost::bind(&LayerCoordinator::reduceSourceTile, shared_from_this<LayerCoordinator>(), tile), REDUCE_PRIO);
}

////////////////////////////////////////////////////////////////////////
/// Helpers

void LayerCoordinator::reduceSourceTile(TileInternal::Ptr tile)
{
  targetTile->initialize();
  const std::pair<int,int> location = sourceTiles[tile];
  const int x = location.first;
  const int y = location.second;

  Tile::Ptr target = targetTile->getTileSync();
  ConstTile::Ptr source = tile->getConstTileSync();

  lo->reduce(target, source, x, y);

  boost::unique_lock<boost::mutex> lock(mut);
  unfinishedSourceTiles--;
  if(!unfinishedSourceTiles)
  {
    targetTile->reportFinished();
  }
}
