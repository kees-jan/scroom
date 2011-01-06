/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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

#include "tileinternal.hh"

#include <string.h>

////////////////////////////////////////////////////////////////////////
/// TileInternalObserver

void TileInternalObserver::tileFinished(TileInternal::Ptr)
{
}

void TileInternalObserver::tileCreated(TileInternal::Ptr)
{
}

////////////////////////////////////////////////////////////////////////
/// TileInternal
TileInternal::TileInternal(int depth, int x, int y, int bpp, TileState state)
  : depth(depth), x(x), y(y), bpp(bpp), state(state), tile(), data(TILESIZE*TILESIZE * bpp / 8)
{
}

TileInternal::Ptr TileInternal::create(int depth, int x, int y, int bpp, TileState state)
{
  TileInternal::Ptr tile = TileInternal::Ptr(new TileInternal(depth, x, y, bpp, state));
  MemoryManager::registerMMI(tile, TILESIZE*TILESIZE * tile->bpp / 8, 1);
  
  return tile;
}

Tile::Ptr TileInternal::getTile()
{
  boost::unique_lock<boost::mutex> lock(mut);
  Tile::Ptr result = tile.lock();
  if(!result && (state == TILE_UNLOADED || state == TILE_LOADED))
  {
    result = Tile::Ptr(new Tile(TILESIZE, TILESIZE, bpp, data.load()));
    tile = result;
    state = TILE_LOADED;
    MemoryManager::loadNotification(shared_from_this<TileInternal>());
  }
  
  return result;
}

void TileInternal::initialize()
{
  boost::unique_lock<boost::mutex> lock(mut);

  if(state == TILE_UNINITIALIZED)
  {
    data.initialize(0);
    state = TILE_LOADED;
    MemoryManager::loadNotification(shared_from_this<TileInternal>());
  }
}
  
void TileInternal::reportFinished()
{
  std::list<TileInternalObserver::Ptr> observers = getObservers();
  while(!observers.empty())
  {
    observers.front()->tileFinished(shared_from_this<TileInternal>());
    observers.pop_front();
  }
}

bool TileInternal::do_unload()
{
  boost::unique_lock<boost::mutex> lock(mut);
  bool isUnloaded = false;
  Tile::Ptr result = tile.lock();
  if(!result && state == TILE_LOADED)
  {
    // Tile not in use. We can unload now...
    data.unload();
    state = TILE_UNLOADED;
    MemoryManager::unloadNotification(shared_from_this<TileInternal>());
    isUnloaded=true;
  }

  return isUnloaded;
}

void TileInternal::observerAdded(TileInternalObserver::Ptr observer)
{
  observer->tileCreated(shared_from_this<TileInternal>());
}
