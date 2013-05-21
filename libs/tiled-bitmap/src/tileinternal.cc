/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2013 Kees-Jan Dijkzeul
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

#include <boost/foreach.hpp>

#include "local.hh"

using namespace Scroom::Utils;
using namespace Scroom::MemoryBlobs;

////////////////////////////////////////////////////////////////////////
/// TileInternal
TileInternal::TileInternal(int depth, int x, int y, int bpp, PageProvider::Ptr provider, TileStateInternal state)
  : depth(depth), x(x), y(y), bpp(bpp), state(state), tile(), provider(provider), data(Blob::create(provider, TILESIZE*TILESIZE * bpp / 8))
{
}

TileInternal::Ptr TileInternal::create(int depth, int x, int y, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider, TileStateInternal state)
{
  TileInternal::Ptr tile = TileInternal::Ptr(new TileInternal(depth, x, y, bpp, provider, state));
  
  return tile;
}

ConstTile::Ptr TileInternal::getConstTileSync()
{
  ConstTile::Ptr result = constTile.lock();
  if(!result)
    result = do_load();
  
  return result;
}

Tile::Ptr TileInternal::getTileSync()
{
  Tile::Ptr result;
  {
	  boost::unique_lock<boost::mutex> lock(tileData);
	  result = tile.lock();
  }
  if(!result)
  {
    // Retrieve the const tile, such that all observers are properly
    // notified of the loading
    ConstTile::Ptr temp = getConstTileSync();
    if(!temp)
    {
    	printf("PANIC: getConstTileSync() didn't return a tile!");
    }
    {
      // Check again. Maybe someone else has beaten us to it...
  	  boost::unique_lock<boost::mutex> lock(tileData);
  	  result = tile.lock();
    }
    if(!result)
    {
      boost::unique_lock<boost::mutex> lock(tileData);
      result = Tile::Ptr(new Tile(TILESIZE, TILESIZE, bpp, data->get()));
      tile = result;
    }
  }
  
  return result;
}

ConstTile::Ptr TileInternal::getConstTileAsync()
{
  ConstTile::Ptr result = constTile.lock();
  return result;
}

Scroom::Utils::Stuff TileInternal::initialize()
{
  Scroom::Utils::Stuff s;
  
  bool didInitialize = false;
  {
    boost::unique_lock<boost::mutex> stateLock(stateData);
    boost::unique_lock<boost::mutex> dataLock(tileData);

    if(state == TSI_UNINITIALIZED)
    {
      s = data->initialize(0);
      state = TSI_NORMAL;
      didInitialize = true;
    }
  }

  if(didInitialize)
    s = do_load(); // Trigger notifyObservers(), without holding the lock

  return s;
}
  
void TileInternal::reportFinished()
{
  TileInternal::Ptr me = shared_from_this<TileInternal>();
  ConstTile::Ptr t = do_load();
  BOOST_FOREACH(TileInitialisationObserver::Ptr observer, Observable<TileInitialisationObserver>::getObservers())
  {
    observer->tileFinished(me);
  }
  BOOST_FOREACH(TileLoadingObserver::Ptr observer, Observable<TileLoadingObserver>::getObservers())
  {
    observer->tileLoaded(t);
  }
}

ConstTile::Ptr TileInternal::do_load()
{
  bool didLoad = false;

  ConstTile::Ptr result;
  {
    boost::unique_lock<boost::mutex> lock(stateData);
    cleanupState();
    state = TSI_LOADING_SYNCHRONOUSLY;
  }
  {
    boost::unique_lock<boost::mutex> lock(tileData);
    result = constTile.lock(); // This ought to fail
    if(!result)
    {
      result = ConstTile::Ptr(new ConstTile(TILESIZE, TILESIZE, bpp, data->getConst()));
      constTile = result;
      didLoad = true;
    }
  }
  {
    boost::unique_lock<boost::mutex> lock(stateData);
    cleanupState();
    state = TSI_NORMAL;
    if(didLoad)
      notifyObservers(result);
  }

  return result;
}

TileViewState::Ptr TileInternal::getViewState(ViewInterface::WeakPtr vi)
{
  TileViewState::Ptr result = viewStates[vi].lock();

  if(!result)
  {
    result = TileViewState::create(shared_from_this<TileInternal>());
    viewStates[vi] = result;
  }

  return result;
}


TileState TileInternal::getState()
{
  TileState result = TILE_UNINITIALIZED;
  switch(state)
  {
  case TSI_NORMAL:
    {
      Tile::Ptr t = tile.lock();
      if(t)
        result = TILE_LOADED;
      else
        result = TILE_UNLOADED;
    }
    break;
  case TSI_OUT_OF_BOUNDS:
    result = TILE_OUT_OF_BOUNDS;
    break;
  case TSI_LOADING_SYNCHRONOUSLY:
  case TSI_LOADING_ASYNCHRONOUSLY:
    result = TILE_UNLOADED;
    break;
  case TSI_UNINITIALIZED:
    result = TILE_UNINITIALIZED;
    break;
  }
  return result;
}

void TileInternal::observerAdded(TileInitialisationObserver::Ptr observer, Scroom::Bookkeeping::Token)
{
  observer->tileCreated(shared_from_this<TileInternal>());
}

void TileInternal::observerAdded(TileLoadingObserver::Ptr observer, Scroom::Bookkeeping::Token token)
{
  ConstTile::Ptr result = constTile.lock();
  ThreadPool::Queue::Ptr queue = this->queue.lock();

  if(!result)
  {
    boost::unique_lock<boost::mutex> lock(stateData);
    cleanupState();
    switch(state)
    {
    case TSI_UNINITIALIZED:
    case TSI_LOADING_ASYNCHRONOUSLY:
    case TSI_LOADING_SYNCHRONOUSLY:
      // Need to do nothing. All will be well.
      break;

    default:
    case TSI_OUT_OF_BOUNDS:
      // Shouldn't happen
      break;

    case TSI_NORMAL:
      // Start an asynchronous load
      if(!queue)
      {
        queue = ThreadPool::Queue::create();
        this->queue = queue;
      }
      CpuBound()->schedule(boost::bind(&TileInternal::do_load, this), LOAD_PRIO, queue);
      state = TSI_LOADING_ASYNCHRONOUSLY;

      break;
    }
  }

  // When the last observer goes away, cancel the load
  if(queue)
    token.add(queue);

  if(result)
    observer->tileLoaded(result);
}

void TileInternal::cleanupState()
{
  if(state == TSI_LOADING_ASYNCHRONOUSLY && !queue.lock())
  {
    // Someone, after triggering an asynchronous load, decided not to need
    // the tile. Changing back to normal.
    state = TSI_NORMAL;
  }
}

void TileInternal::notifyObservers(ConstTile::Ptr tile)
{
  BOOST_FOREACH(TileLoadingObserver::Ptr observer, Observable<TileLoadingObserver>::getObservers())
  {
    observer->tileLoaded(tile);
  }
}

void TileInternal::open(ViewInterface::WeakPtr)
{
  // On open, we do nothing. On close, we destroy any resources related to the view.
}

void TileInternal::close(ViewInterface::WeakPtr vi)
{
  viewStates.erase(vi);
}