/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

////////////////////////////////////////////////////////////////////////
/// TileInternal
TileInternal::TileInternal(int depth, int x, int y, int bpp, TileStateInternal state)
  : depth(depth), x(x), y(y), bpp(bpp), state(state), tile(), data(TILESIZE*TILESIZE * bpp / 8)
{
}

TileInternal::Ptr TileInternal::create(int depth, int x, int y, int bpp, TileStateInternal state)
{
  TileInternal::Ptr tile = TileInternal::Ptr(new TileInternal(depth, x, y, bpp, state));
  tile->performMemoryManagerRegistration();
  
  return tile;
}

void TileInternal::performMemoryManagerRegistration()
{
  memoryManagerRegistration = MemoryManager::registerMMI(shared_from_this<MemoryManagedInterface>(),
                                                         TILESIZE*TILESIZE * bpp / 8, 0);
}

Tile::Ptr TileInternal::getTileSync()
{
  Tile::Ptr result = tile.lock();
  if(!result)
    result = do_load();
  
  return result;
}

Tile::Ptr TileInternal::getTileAsync()
{
  Tile::Ptr result = tile.lock();
  if(!result && state == TSI_LOADED)
    result = do_load();

  return result;
}

void TileInternal::initialize()
{
  bool didInitialize = false;
  {
    boost::unique_lock<boost::mutex> stateLock(stateData);
    boost::unique_lock<boost::mutex> dataLock(tileData);

    if(state == TSI_UNINITIALIZED)
    {
      data.initialize(0);
      state = TSI_LOADED;
      didInitialize = true;
      MemoryManager::loadNotification(memoryManagerRegistration);
    }
  }

  if(didInitialize)
    do_load(); // Trigger notifyObservers(), without holding the lock
}
  
void TileInternal::reportFinished()
{
  BOOST_FOREACH(TileInitialisationObserver::Ptr observer, Observable<TileInitialisationObserver>::getObservers())
  {
    observer->tileFinished(shared_from_this<TileInternal>());
  }
  BOOST_FOREACH(TileLoadingObserver::Ptr observer, Observable<TileLoadingObserver>::getObservers())
  {
    observer->tileLoaded(do_load());
  }
}

bool TileInternal::do_unload()
{
  boost::unique_lock<boost::mutex> stateLock(stateData);
  boost::unique_lock<boost::mutex> dataLock(tileData);
  cleanupState();

  Tile::Ptr result = tile.lock();
  if(!result && state == TSI_LOADED)
  {
    // Tile not in use. We can unload now...
    data.unload();
    state = TSI_UNLOADED;
    MemoryManager::unloadNotification(memoryManagerRegistration);
  }
  else if (result)
  {
    printf("Tile in use. Refusing to unload %d Mb\n", TILESIZE*TILESIZE*bpp/8/1024/1024);
    MemoryManager::loadNotification(memoryManagerRegistration);
  }

  return state == TSI_UNLOADED;
}

Tile::Ptr TileInternal::do_load()
{
  bool didLoad = false;

  Tile::Ptr result;
  {
    boost::unique_lock<boost::mutex> lock(stateData);
    cleanupState();
    state = TSI_LOADING_SYNCHRONOUSLY;
  }
  {
    boost::unique_lock<boost::mutex> lock(tileData);
    result = tile.lock(); // This ought to fail
    if(!result)
    {
      result = Tile::Ptr(new Tile(TILESIZE, TILESIZE, bpp, data.load()));
      tile = result;
      MemoryManager::loadNotification(memoryManagerRegistration);
      didLoad = true;
    }
  }
  {
    boost::unique_lock<boost::mutex> lock(stateData);
    cleanupState();
    state = TSI_LOADED;
    if(didLoad)
      notifyObservers(result);
  }

  return result;
}

TileViewState::Ptr TileInternal::getViewState(ViewInterface* vi)
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
  case TSI_LOADED:
    result = TILE_LOADED;
    break;
  case TSI_OUT_OF_BOUNDS:
    result = TILE_OUT_OF_BOUNDS;
    break;
  case TSI_UNLOADED:
  case TSI_LOADING_SYNCHRONOUSLY:
  case TSI_LOADING_ASYNCHRONOUSLY:
    result = TILE_UNLOADED;
    break;
  case TSI_UNINITIALIZED:
  default:
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
  Tile::Ptr result = tile.lock();
  ThreadPool::Queue::Ptr queue = this->queue.lock();

  if(!result)
  {
    boost::unique_lock<boost::mutex> lock(stateData);
    cleanupState();
    switch(state)
    {
    case TSI_LOADED:
    case TSI_LOADING_SYNCHRONOUSLY:
    case TSI_LOADING_ASYNCHRONOUSLY:
    case TSI_UNINITIALIZED:
      // Need to do nothing. All will be well.
      break;

    default:
    case TSI_OUT_OF_BOUNDS:
      // Shouldn't happen
      break;

    case TSI_UNLOADED:
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
    // the tile. Changing back to unloaded.
    state = TSI_UNLOADED;
  }
}

void TileInternal::notifyObservers(Tile::Ptr tile)
{
  BOOST_FOREACH(TileLoadingObserver::Ptr observer, Observable<TileLoadingObserver>::getObservers())
    {
      observer->tileLoaded(tile);
    }
}

void TileInternal::open(ViewInterface*)
{
  // On open, we do nothing. On close, we destroy any resources related to the view.
}

void TileInternal::close(ViewInterface* vi)
{
  viewStates.erase(vi);
}
