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

#include <scroom/workinterface.hh>
#include <scroom/threadpool.hh>

class TileReducer : public WorkInterface
{
private:
  LayerOperations* lo;
  TileInternal* targetTile;
  TileInternal* sourceTile;
  int x;
  int y;
  boost::mutex& mut;
  int& unfinishedSourceTiles;
  
public:
  TileReducer(LayerOperations* lo,
              TileInternal* targetTile, TileInternal* sourceTile,
              int x, int y,
              boost::mutex& mut, int& unfinishedSourceTiles);

  // WorkInterface ///////////////////////////////////////////////////////
  virtual bool doWork();
};

////////////////////////////////////////////////////////////////////////

LayerCoordinator::LayerCoordinator(TileInternal* targetTile,
                                   LayerOperations* lo)
  : targetTile(targetTile), lo(lo), unfinishedSourceTiles(0)
{
}

LayerCoordinator::~LayerCoordinator()
{
  std::map<TileInternal*,std::pair<int,int> >::iterator cur =
    sourceTiles.begin();
  std::map<TileInternal*,std::pair<int,int> >::iterator end =
    sourceTiles.end();

  for(;cur!=end;cur++)
  {
    cur->first->unregisterObserver(this);
  }
  sourceTiles.clear();
}

void LayerCoordinator::addSourceTile(int x, int y, TileInternal* tile)
{
  boost::unique_lock<boost::mutex> lock(mut);

  sourceTiles[tile] = std::make_pair(x,y);
  tile->registerObserver(this);
  unfinishedSourceTiles++;
}

////////////////////////////////////////////////////////////////////////
/// TileInternalObserver

void LayerCoordinator::tileFinished(TileInternal* tile)
{
  targetTile->initialize();
  std::pair<int,int> location = sourceTiles[tile];

  TileReducer* tr = new TileReducer(lo, targetTile, tile,
                                    location.first, location.second,
                                    mut, unfinishedSourceTiles);

  schedule(tr, PRIO_LOW);
}

////////////////////////////////////////////////////////////////////////
/// TileReducer

TileReducer::TileReducer(LayerOperations* lo,
                         TileInternal* targetTile, TileInternal* sourceTile,
                         int x, int y,
                         boost::mutex& mut, int& unfinishedSourceTiles)
  : lo(lo), targetTile(targetTile), sourceTile(sourceTile),
    x(x), y(y), mut(mut), unfinishedSourceTiles(unfinishedSourceTiles)
{
}

bool TileReducer::doWork()
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
  return false;
}
