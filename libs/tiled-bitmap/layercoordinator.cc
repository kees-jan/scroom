/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

class TileReducer
{
private:
  LayerOperations* lo;
  TileInternal::Ptr targetTile;
  TileInternal::Ptr sourceTile;
  int x;
  int y;
  boost::mutex& mut;
  int& unfinishedSourceTiles;
  
public:
  TileReducer(LayerOperations* lo,
              TileInternal::Ptr targetTile, TileInternal::Ptr sourceTile,
              int x, int y,
              boost::mutex& mut, int& unfinishedSourceTiles);

  void operator()();
};

////////////////////////////////////////////////////////////////////////

LayerCoordinator::LayerCoordinator(TileInternal::Ptr targetTile,
                                   LayerOperations* lo)
  : targetTile(targetTile), lo(lo), unfinishedSourceTiles(0)
{
}

LayerCoordinator::~LayerCoordinator()
{
  std::map<TileInternal::Ptr,std::pair<int,int> >::iterator cur =
    sourceTiles.begin();
  std::map<TileInternal::Ptr,std::pair<int,int> >::iterator end =
    sourceTiles.end();

  for(;cur!=end;cur++)
  {
    cur->first->unregisterObserver(this);
  }
  sourceTiles.clear();
}

void LayerCoordinator::addSourceTile(int x, int y, TileInternal::Ptr tile)
{
  boost::unique_lock<boost::mutex> lock(mut);

  sourceTiles[tile] = std::make_pair(x,y);
  tile->registerObserver(this);
  unfinishedSourceTiles++;
}

////////////////////////////////////////////////////////////////////////
/// TileInternalObserver

void LayerCoordinator::tileFinished(TileInternal::Ptr tile)
{
  targetTile->initialize();
  std::pair<int,int> location = sourceTiles[tile];

  TileReducer tr(lo, targetTile, tile, location.first, location.second, mut, unfinishedSourceTiles);

  CpuBound::schedule(tr, REDUCE_PRIO);
}

////////////////////////////////////////////////////////////////////////
/// TileReducer

TileReducer::TileReducer(LayerOperations* lo,
                         TileInternal::Ptr targetTile, TileInternal::Ptr sourceTile,
                         int x, int y,
                         boost::mutex& mut, int& unfinishedSourceTiles)
  : lo(lo), targetTile(targetTile), sourceTile(sourceTile),
    x(x), y(y), mut(mut), unfinishedSourceTiles(unfinishedSourceTiles)
{
}

void TileReducer::operator()()
{
  Tile::Ptr target = targetTile->getTile();
  Tile::Ptr source = sourceTile->getTile();

  lo->reduce(target, source, x, y);

  boost::unique_lock<boost::mutex> lock(mut);
  unfinishedSourceTiles--;
  if(!unfinishedSourceTiles)
  {
    targetTile->reportFinished();
  }
}
