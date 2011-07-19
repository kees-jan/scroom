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
#include "tileviewstate.hh"

#include <boost/foreach.hpp>

#include <scroom/unused.h>

#include "local.hh"
#include "tileinternal.hh"
#include "tiledbitmapviewdata.hh"

TileViewState::~TileViewState()
{
  r.reset();
}

TileViewState::Ptr TileViewState::create(boost::shared_ptr<TileInternal> parent)
{
  TileViewState::Ptr result(new TileViewState(parent));

  result->r = parent->registerObserver(result);

  return result;
}

TileViewState::TileViewState(boost::shared_ptr<TileInternal> parent)
  : parent(parent), state(INIT), desiredState(BASE_COMPUTED), lo(NULL), cpuBound(CpuBound())
{
}

void TileViewState::tileLoaded(Tile::Ptr tile)
{
  boost::mutex::scoped_lock l(mut);
  this->tile = tile;

  kick();
}

Scroom::Utils::Registration TileViewState::getCacheResult()
{
  boost::mutex::scoped_lock l(mut);
  return zoomCache.lock();
}

void TileViewState::setViewData(TiledBitmapViewData::Ptr tbvd)
{
  boost::mutex::scoped_lock l(mut);
  this->tbvd = tbvd;
  kick();
}

void TileViewState::setZoom(LayerOperations* lo, int zoom)
{
  bool mustKick = false;
  
  boost::mutex::scoped_lock l(mut);
  if(zoom != this->zoom || desiredState != DONE)
  {
    this->zoom = zoom;
    this->lo = lo;
    this->desiredState = DONE;
    mustKick = true;

    // Abort any zoom currently in progress
    if(state>=BASE_COMPUTED)
    {
      queue.reset();
      zoomCache.reset();
      state = BASE_COMPUTED;
    }
  }

  if(mustKick)
    kick();
}


void TileViewState::kick()
{
  // Cleanup...
  if(state>=ZOOM_COMPUTED && !zoomCache.lock())
  {
    queue.reset();
    state = BASE_COMPUTED;
  }

  if(state>=BASE_COMPUTED && !baseCache.lock())
  {
    queue.reset();
    state = LOADED;
  }
  
  if(state>=LOADED && !tile)
  {
    printf("PANIC: State LOADED and no tile shouldn't happen.\n");
    queue.reset();
    state = INIT;
  }

  ThreadPool::Queue::Ptr queue = this->queue.lock();
  TiledBitmapViewData::Ptr tbvd = this->tbvd.lock();
  if(state >= LOADED && desiredState >= state && !queue && tbvd)
  {
    queue = ThreadPool::Queue::createAsync();
    this->queue = queue;
    this->weakQueue = queue->getWeak();
    tbvd->storeVolatileStuff(queue);

    cpuBound->schedule(boost::bind(&TileViewState::process, shared_from_this<TileViewState>(), this->weakQueue.lock()),
                       LOAD_PRIO, queue);
  }
}

void TileViewState::process(ThreadPool::WeakQueue::Ptr wq)
{
  for(;;)
  {
    boost::function<void ()> fn;

    {
      boost::mutex::scoped_lock l(mut);
      if(wq != weakQueue.lock())
      {
        // Someone deleted the queue I'm executing on. Abort
        return;
      }
      if(state>=desiredState)
      {
        // Desired state reached. Abort.
        return;
      }

      switch(state)
      {
      case LOADED:
        fn = boost::bind(&TileViewState::computeBase, shared_from_this<TileViewState>(), wq, tile, lo);
        state = COMPUTING_BASE;
        break;
        
      case BASE_COMPUTED:
        fn = boost::bind(&TileViewState::computeZoom, shared_from_this<TileViewState>(), wq, tile, lo, baseCache.lock(), zoom);
        state = COMPUTING_ZOOM;
        break;

      case ZOOM_COMPUTED:
        state = DONE;
        fn = boost::bind(&TileViewState::reportDone, shared_from_this<TileViewState>(), wq, tile);
        return;

      case INIT:
      case COMPUTING_BASE:
      case COMPUTING_ZOOM:
      case DONE:
      default:
        printf("PANIC: Don't know what to do in state %d\n", state);
        return;
      }
    }

    fn();
  }
}

void TileViewState::computeBase(ThreadPool::WeakQueue::Ptr wq, Tile::Ptr tile, LayerOperations* lo)
{
  Scroom::Utils::Registration baseCache = lo->cache(tile);

  boost::mutex::scoped_lock l(mut);
  TiledBitmapViewData::Ptr tbvd = this->tbvd.lock();
  if(tbvd && desiredState>=BASE_COMPUTED && this->weakQueue.lock() == wq)
  {
    this->baseCache = baseCache;
    tbvd->storeVolatileStuff(baseCache);
    state=BASE_COMPUTED;
  }
}

void TileViewState::computeZoom(ThreadPool::WeakQueue::Ptr wq, Tile::Ptr tile, LayerOperations* lo, Scroom::Utils::Registration baseCache, int zoom)
{
  Scroom::Utils::Registration zoomCache = lo->cache(tile, zoom, baseCache);

  boost::mutex::scoped_lock l(mut);
  TiledBitmapViewData::Ptr tbvd = this->tbvd.lock();
  if(tbvd && desiredState>=ZOOM_COMPUTED && this->zoom == zoom && this->weakQueue.lock() == wq)
  {
    this->zoomCache = zoomCache;
    tbvd->storeVolatileStuff(zoomCache);
    state=ZOOM_COMPUTED;
  }
}

void TileViewState::reportDone(ThreadPool::WeakQueue::Ptr wq, Tile::Ptr tile)
{
  UNUSED(wq);
    
  BOOST_FOREACH(TileLoadingObserver::Ptr observer, Scroom::Utils::Observable<TileLoadingObserver>::getObservers())
  {
    observer->tileLoaded(tile);
  }
}

