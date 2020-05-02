/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "tileviewstate.hh"

#include <scroom/unused.hh>

#include "local.hh"
#include "tiledbitmapviewdata.hh"

TileViewState::~TileViewState()
{
  r.reset();
}

TileViewState::Ptr TileViewState::create(boost::shared_ptr<CompressedTile> parent)
{
  TileViewState::Ptr result(new TileViewState(parent));

  result->r = parent->registerObserver(result);

  return result;
}

TileViewState::TileViewState(boost::shared_ptr<CompressedTile> parent_)
  : parent(parent_), state(INIT), desiredState(LOADED), lo(), zoom(0), cpuBound(CpuBound())
{
}

void TileViewState::tileLoaded(ConstTile::Ptr tile_)
{
  boost::mutex::scoped_lock l(mut);

  this->tile = tile_;

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

void TileViewState::setViewData(TiledBitmapViewData::Ptr tbvd_)
{
  boost::mutex::scoped_lock l(mut);
  this->tbvd = tbvd_;

  Scroom::Utils::Stuff lifeTimeManager_ = this->lifeTimeManager.lock();
  if(!lifeTimeManager_)
  {
    lifeTimeManager_ = boost::shared_ptr<void>(reinterpret_cast<void*>(0xDEAD),
                                              boost::bind(&TileViewState::clear,
                                                          shared_from_this<TileViewState>()));
    this->lifeTimeManager = lifeTimeManager_;
  }
  tbvd_->storeVolatileStuff(lifeTimeManager_);

  kick();
}

void TileViewState::setZoom(LayerOperations::Ptr lo_, int zoom_)
{
  bool mustKick = false;

  boost::mutex::scoped_lock l(mut);
  if(desiredState != DONE || zoom_ != this->zoom)
  {
    this->zoom = zoom_;
    this->lo = lo_;
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

  TiledBitmapViewData::Ptr tbvd_ = this->tbvd.lock();

  // printf("kick: Tile %d, %d, %d: state=%d, desired=%d, queue=%d, tbvd=%d\n",
  //        parent->depth, parent->x, parent->y, state, desiredState, (bool)queue, (bool)tbvd);
  if(state >= LOADED && desiredState >= state && !queue && tbvd_)
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

void TileViewState::computeBase(ThreadPool::WeakQueue::Ptr wq, ConstTile::Ptr tile_, LayerOperations::Ptr lo_)
{
  Scroom::Utils::Stuff baseCache_ = lo_->cache(tile_);

  boost::mutex::scoped_lock l(mut);
  TiledBitmapViewData::Ptr tbvd_ = this->tbvd.lock();
  // printf("computeBase: Tile %d, %d, %d: state=%d, desired=%d, tbvd=%d, queue? %d\n",
  //        parent->depth, parent->x, parent->y, state, desiredState, (bool)tbvd, (bool)(wq==weakQueue));

  if(tbvd_ && desiredState>=BASE_COMPUTED && this->weakQueue == wq)
  {
    this->baseCache = baseCache_;
    state=BASE_COMPUTED;
  }
}

void TileViewState::computeZoom(ThreadPool::WeakQueue::Ptr wq, ConstTile::Ptr tile_, LayerOperations::Ptr lo_, Scroom::Utils::Stuff baseCache_, int zoom_)
{
  Scroom::Utils::Stuff zoomCache_ = lo_->cacheZoom(tile_, zoom_, baseCache_);

  boost::mutex::scoped_lock l(mut);
  TiledBitmapViewData::Ptr tbvd_ = this->tbvd.lock();
  // printf("computeZoom: Tile %d, %d, %d: state=%d(%d), zoom=%d(%d), tbvd=%d, queue? %d\n",
  //        parent->depth, parent->x, parent->y, state, desiredState, zoom, this->zoom, (bool)tbvd, (bool)(wq==weakQueue));
  if(tbvd_ && desiredState>=ZOOM_COMPUTED && this->zoom == zoom_ && this->weakQueue == wq)
  {
    this->zoomCache = zoomCache_;
    state=ZOOM_COMPUTED;
  }
}

void TileViewState::reportDone(ThreadPool::WeakQueue::Ptr wq, ConstTile::Ptr tile_)
{
  UNUSED(wq);

  // printf("reportDone: Tile %d, %d, %d: state=%d, desired=%d\n",
  //        parent->depth, parent->x, parent->y, state, desiredState);

  for(TileLoadingObserver::Ptr observer: Scroom::Utils::Observable<TileLoadingObserver>::getObservers())
  {
    observer->tileLoaded(tile_);
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
