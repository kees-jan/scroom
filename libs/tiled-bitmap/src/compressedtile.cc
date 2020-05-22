/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "scroom/tiledbitmaplayer.hh"

#include <string.h>

#include "local.hh"
#include "tileviewstate.hh"

using namespace Scroom::Utils;
using namespace Scroom::MemoryBlobs;

////////////////////////////////////////////////////////////////////////
/// CompressedTile
CompressedTile::CompressedTile(int depth_, int x_, int y_, int bpp_, PageProvider::Ptr provider_, TileStateInternal state_)
  : depth(depth_), x(x_), y(y_), bpp(bpp_), state(state_), tile(), provider(provider_), data(Blob::create(provider_, TILESIZE*TILESIZE * bpp / 8))
{
}

CompressedTile::Ptr CompressedTile::create(int depth, int x, int y, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider, TileStateInternal state)
{
  CompressedTile::Ptr tile = CompressedTile::Ptr(new CompressedTile(depth, x, y, bpp, provider, state));

  return tile;
}

ConstTile::Ptr CompressedTile::getConstTileSync()
{
  ConstTile::Ptr result = constTile.lock();
  if(!result)
    result = do_load();

  return result;
}

Tile::Ptr CompressedTile::getTileSync()
{
  Tile::Ptr result;
  {
    boost::mutex::scoped_lock lock(tileData);
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
      boost::mutex::scoped_lock lock(tileData);
      result = tile.lock();
    }
    if(!result)
    {
      boost::mutex::scoped_lock lock(tileData);
      result = Tile::Ptr(new Tile(TILESIZE, TILESIZE, bpp, data->get()));
      tile = result;
    }
  }

  return result;
}

ConstTile::Ptr CompressedTile::getConstTileAsync()
{
  ConstTile::Ptr result = constTile.lock();
  return result;
}

Tile::Ptr CompressedTile::initialize()
{
  Scroom::Utils::Stuff s;
  Tile::Ptr tile_;

  bool didInitialize = false;
  {
    boost::mutex::scoped_lock stateLock(stateData);
    boost::mutex::scoped_lock dataLock(tileData);

    if(state == TSI_UNINITIALIZED)
    {
      s = data->initialize(0);
      state = TSI_NORMAL;
      didInitialize = true;
    }
  }

  if(didInitialize)
    tile_ = getTileSync(); // Trigger notifyObservers(), without holding the lock

  return tile_;
}

void CompressedTile::reportFinished()
{
  CompressedTile::Ptr me = shared_from_this<CompressedTile>();
  ConstTile::Ptr t = do_load();
  for(TileInitialisationObserver::Ptr observer: Observable<TileInitialisationObserver>::getObservers())
  {
    observer->tileFinished(me);
  }
  for(TileLoadingObserver::Ptr observer: Observable<TileLoadingObserver>::getObservers())
  {
    observer->tileLoaded(t);
  }
}

ConstTile::Ptr CompressedTile::do_load()
{
  bool didLoad = false;

  ConstTile::Ptr result;
  {
    boost::mutex::scoped_lock lock(stateData);
    cleanupState();
    state = TSI_LOADING_SYNCHRONOUSLY;
  }
  {
    boost::mutex::scoped_lock lock(tileData);
    result = constTile.lock(); // This ought to fail
    if(!result)
    {
      result = ConstTile::Ptr(new ConstTile(TILESIZE, TILESIZE, bpp, data->getConst()));
      constTile = result;
      didLoad = true;
    }
  }
  {
    boost::mutex::scoped_lock lock(stateData);
    cleanupState();
    state = TSI_NORMAL;
    if(didLoad)
      notifyObservers(result);
  }

  return result;
}

TileViewState::Ptr CompressedTile::getViewState(ViewInterface::WeakPtr vi)
{
  TileViewState::Ptr result = viewStates[vi].lock();

  if(!result)
  {
    result = TileViewState::create(shared_from_this<CompressedTile>());
    viewStates[vi] = result;
  }

  return result;
}

TileState CompressedTile::getState()
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

void CompressedTile::observerAdded(TileInitialisationObserver::Ptr const& observer, Scroom::Bookkeeping::Token const&)
{
  observer->tileCreated(shared_from_this<CompressedTile>());
}

void CompressedTile::observerAdded(TileLoadingObserver::Ptr const& observer, Scroom::Bookkeeping::Token const& token)
{
  ConstTile::Ptr result = constTile.lock();
  ThreadPool::Queue::Ptr queue_ = this->queue.lock();

  if(!result)
  {
    boost::mutex::scoped_lock lock(stateData);
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
      if(!queue_)
      {
        queue_ = ThreadPool::Queue::create();
        this->queue = queue_;
      }
      CpuBound()->schedule(boost::bind(&CompressedTile::do_load, this), LOAD_PRIO, queue_);
      state = TSI_LOADING_ASYNCHRONOUSLY;

      break;
    }
  }

  // When the last observer goes away, cancel the load
  if(queue_)
    token.add(queue_);

  if(result)
    observer->tileLoaded(result);
}

void CompressedTile::cleanupState()
{
  if(state == TSI_LOADING_ASYNCHRONOUSLY && !queue.lock())
  {
    // Someone, after triggering an asynchronous load, decided not to need
    // the tile. Changing back to normal.
    state = TSI_NORMAL;
  }
}

void CompressedTile::notifyObservers(ConstTile::Ptr tile_)
{
  for(TileLoadingObserver::Ptr observer: Observable<TileLoadingObserver>::getObservers())
  {
    observer->tileLoaded(tile_);
  }
}

void CompressedTile::open(ViewInterface::WeakPtr)
{
  // On open, we do nothing. On close, we destroy any resources related to the view.
}

void CompressedTile::close(ViewInterface::WeakPtr vi)
{
  viewStates.erase(vi);
}

////////////////////////////////////////////////////////////////////////
/// TileInitialisationObserver

void TileInitialisationObserver::tileFinished(CompressedTile::Ptr)
{
}

void TileInitialisationObserver::tileCreated(CompressedTile::Ptr)
{
}

