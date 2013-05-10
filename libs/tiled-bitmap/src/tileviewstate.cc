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
  : parent(parent), state(INIT), desiredState(LOADED), lo(), zoom(0), cpuBound(CpuBound())
{
}

void TileViewState::tileLoaded(ConstTile::Ptr tile)
{
  boost::mutex::scoped_lock l(mut);

  this->tile = tile;

  // Abort any zoom currently in progress
  if(state>LOADED)
  {
    queue.reset();
    weakQueue.reset();
    zoomCache.reset();
  }
  state = LOADED;

  kick();
}

Scroom::Utils::Stuff TileViewState::getCacheResult()
{
  boost::mutex::scoped_lock l(mut);
  return zoomCache;
}

void TileViewState::setViewData(TiledBitmapViewData::Ptr tbvd)
{
  boost::mutex::scoped_lock l(mut);
  this->tbvd = tbvd;

  Scroom::Utils::Stuff lifeTimeManager = this->lifeTimeManager.lock();
  if(!lifeTimeManager)
  {
    lifeTimeManager = boost::shared_ptr<void>((void*)0xDEAD,
                                              boost::bind(&TileViewState::clear,
                                                          shared_from_this<TileViewState>()));
    this->lifeTimeManager = lifeTimeManager;
  }
  tbvd->storeVolatileStuff(lifeTimeManager);

  kick();
}

void TileViewState::setZoom(LayerOperations::Ptr lo, int zoom)
{
  bool mustKick = false;
  
  boost::mutex::scoped_lock l(mut);
  if(desiredState != DONE || zoom != this->zoom)
  {
    this->zoom = zoom;
    this->lo = lo;
    this->desiredState = DONE;
    mustKick = true;

    // Abort any zoom currently in progress
    if(state>=BASE_COMPUTED)
    {
      queue.reset();
      weakQueue.reset();
      zoomCache.reset();
      state = BASE_COMPUTED;
    }
  }

  if(mustKick)
    kick();
}

void TileViewState::kick()
{
  if(state>=LOADED && !tile)
  {
    printf("PANIC: State LOADED and no tile shouldn't happen.\n");
    queue.reset();
    weakQueue.reset();
    state = INIT;
  }

  TiledBitmapViewData::Ptr tbvd = this->tbvd.lock();

  // printf("kick: Tile %d, %d, %d: state=%d, desired=%d, queue=%d, tbvd=%d\n",
  //        parent->depth, parent->x, parent->y, state, desiredState, (bool)queue, (bool)tbvd);
  if(state >= LOADED && desiredState >= state && !queue && tbvd)
  {
    // printf("kick: Tile %d, %d, %d: Starting processing\n", parent->depth, parent->x, parent->y);

    queue = ThreadPool::Queue::createAsync();
    weakQueue = queue->getWeak();

    cpuBound->schedule(boost::bind(&TileViewState::process, shared_from_this<TileViewState>(), weakQueue),
                       LOAD_PRIO, queue);
  }
}

void TileViewState::process(ThreadPool::WeakQueue::Ptr wq)
{
  for(;;)
  {
    boost::mutex::scoped_lock l(mut);
    // printf("process: Tile %d, %d, %d: state=%d, desired=%d, queue? %d\n",
    //        parent->depth, parent->x, parent->y, state, desiredState, (bool)(wq==weakQueue));
      
    if(wq == weakQueue && state < desiredState)
    {
      boost::function<void ()> fn;
        
      switch(state)
      {
      case LOADED:
        fn = boost::bind(&TileViewState::computeBase, shared_from_this<TileViewState>(), wq, tile, lo);
        state = COMPUTING_BASE;
        break;
        
      case BASE_COMPUTED:
        fn = boost::bind(&TileViewState::computeZoom, shared_from_this<TileViewState>(),
                         wq, tile, lo, baseCache, zoom);
        state = COMPUTING_ZOOM;
        break;

      case ZOOM_COMPUTED:
        state = DONE;
        fn = boost::bind(&TileViewState::reportDone, shared_from_this<TileViewState>(), wq, tile);
        break;

      case INIT:
      case COMPUTING_BASE:
      case COMPUTING_ZOOM:
      case DONE:
      default:
        printf("PANIC: Don't know what to do in state %d\n", state);
        return;
      }

      l.unlock();
      fn();
    }
    else
    {
      break;
    }

  }


  {
    boost::mutex::scoped_lock l(mut);
      
    if(wq == weakQueue)
    {
      queue.reset();
      weakQueue.reset();
    }
  }
}

void TileViewState::computeBase(ThreadPool::WeakQueue::Ptr wq, ConstTile::Ptr tile, LayerOperations::Ptr lo)
{
  Scroom::Utils::Stuff baseCache = lo->cache(tile);

  boost::mutex::scoped_lock l(mut);
  TiledBitmapViewData::Ptr tbvd = this->tbvd.lock();
  // printf("computeBase: Tile %d, %d, %d: state=%d, desired=%d, tbvd=%d, queue? %d\n",
  //        parent->depth, parent->x, parent->y, state, desiredState, (bool)tbvd, (bool)(wq==weakQueue));
  
  if(tbvd && desiredState>=BASE_COMPUTED && this->weakQueue == wq)
  {
    this->baseCache = baseCache;
    state=BASE_COMPUTED;
  }
}

void TileViewState::computeZoom(ThreadPool::WeakQueue::Ptr wq, ConstTile::Ptr tile, LayerOperations::Ptr lo, Scroom::Utils::Stuff baseCache, int zoom)
{
  Scroom::Utils::Stuff zoomCache = lo->cacheZoom(tile, zoom, baseCache);

  boost::mutex::scoped_lock l(mut);
  TiledBitmapViewData::Ptr tbvd = this->tbvd.lock();
  // printf("computeZoom: Tile %d, %d, %d: state=%d(%d), zoom=%d(%d), tbvd=%d, queue? %d\n",
  //        parent->depth, parent->x, parent->y, state, desiredState, zoom, this->zoom, (bool)tbvd, (bool)(wq==weakQueue));
  if(tbvd && desiredState>=ZOOM_COMPUTED && this->zoom == zoom && this->weakQueue == wq)
  {
    this->zoomCache = zoomCache;
    state=ZOOM_COMPUTED;
  }
}

void TileViewState::reportDone(ThreadPool::WeakQueue::Ptr wq, ConstTile::Ptr tile)
{
  UNUSED(wq);
    
  // printf("reportDone: Tile %d, %d, %d: state=%d, desired=%d\n",
  //        parent->depth, parent->x, parent->y, state, desiredState);

  BOOST_FOREACH(TileLoadingObserver::Ptr observer, Scroom::Utils::Observable<TileLoadingObserver>::getObservers())
  {
    observer->tileLoaded(tile);
  }
}


void TileViewState::clear()
{
  // printf("Clearing viewstate for tile %d, %d, %d\n", parent->depth, parent->x, parent->y);
  boost::mutex::scoped_lock l(mut);

  if(state >= LOADED)
    state = LOADED;

  queue.reset();
  weakQueue.reset();
  lifeTimeManager.reset();
  baseCache.reset();
  zoomCache.reset();

  kick();
}
