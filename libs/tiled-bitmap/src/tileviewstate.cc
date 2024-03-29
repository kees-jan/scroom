/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "tileviewstate.hh"

#include <cstdio>
#include <utility>

#include <fmt/format.h>

#include <scroom/observable.hh>
#include <scroom/stuff.hh>
#include <scroom/threadpool.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/tiledbitmaplayer.hh>

#include "local.hh"
#include "tiledbitmapviewdata.hh"

TileViewState::~TileViewState() { r.reset(); }

TileViewState::Ptr TileViewState::create(const std::shared_ptr<CompressedTile>& parent)
{
  TileViewState::Ptr result(new TileViewState(parent));

  result->r = parent->registerObserver(result);

  return result;
}

TileViewState::TileViewState(std::shared_ptr<CompressedTile> parent_)
  : parent(std::move(parent_))
  , cpuBound(CpuBound())
{
}

void TileViewState::tileLoaded(ConstTile::Ptr tile_)
{
  boost::mutex::scoped_lock const l(mut);

  tile = tile_;

  // Abort any zoom currently in progress
  if(state > LOADED)
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
  boost::mutex::scoped_lock const l(mut);
  return zoomCache;
}

void TileViewState::setViewData(const TiledBitmapViewData::Ptr& tbvd_)
{
  boost::mutex::scoped_lock const l(mut);
  tbvd = tbvd_;

  Scroom::Utils::Stuff lifeTimeManager_ = lifeTimeManager.lock();
  if(!lifeTimeManager_)
  {
    lifeTimeManager_ = std::shared_ptr<void>(reinterpret_cast<void*>(0xDEAD),
                                             [me = shared_from_this<TileViewState>()](auto p [[maybe_unused]]) { me->clear(); });
    lifeTimeManager  = lifeTimeManager_;
  }
  tbvd_->storeVolatileStuff(lifeTimeManager_);

  kick();
}

void TileViewState::setZoom(LayerOperations::Ptr lo_, int zoom_)
{
  bool mustKick = false;

  boost::mutex::scoped_lock const l(mut);
  if(desiredState != DONE || zoom_ != zoom)
  {
    zoom         = zoom_;
    lo           = std::move(lo_);
    desiredState = DONE;
    mustKick     = true;

    // Abort any zoom currently in progress
    if(state >= BASE_COMPUTED)
    {
      queue.reset();
      weakQueue.reset();
      zoomCache.reset();
      state = BASE_COMPUTED;
    }
  }

  if(mustKick)
  {
    kick();
  }
}

void TileViewState::kick()
{
  if(state >= LOADED && !tile)
  {
    defect_message("PANIC: State LOADED and no tile shouldn't happen.");
  }

  TiledBitmapViewData::Ptr const tbvd_ = tbvd.lock();

  if(state >= LOADED && desiredState >= state && !queue && tbvd_)
  {
    queue = ThreadPool::Queue::createAsync();

    cpuBound->schedule(
      [me = shared_from_this<TileViewState>(), weakQueue = queue->getWeak()] { me->process(weakQueue); }, LOAD_PRIO, queue);
  }
}

void TileViewState::process(const ThreadPool::WeakQueue::Ptr& wq)
{
  for(;;)
  {
    boost::mutex::scoped_lock l(mut);

    if(wq == weakQueue && state < desiredState)
    {
      boost::function<void()> fn;

      switch(state)
      {
      case LOADED:
        fn    = [me = shared_from_this<TileViewState>(), wq, tile = tile, lo = lo] { me->computeBase(wq, tile, lo); };
        state = COMPUTING_BASE;
        break;

      case BASE_COMPUTED:
        fn = [me = shared_from_this<TileViewState>(), wq, tile = tile, lo = lo, baseCache = baseCache, zoom = zoom]
        { me->computeZoom(wq, tile, lo, baseCache, zoom); };
        state = COMPUTING_ZOOM;
        break;

      case ZOOM_COMPUTED:
        state = DONE;
        fn    = [me = shared_from_this<TileViewState>(), wq, tile = tile] { me->reportDone(wq, tile); };
        break;

      case INIT:
      case COMPUTING_BASE:
      case COMPUTING_ZOOM:
      case DONE:
      default:
        defect_message(fmt::format("PANIC: Don't know what to do in state {}", static_cast<int>(state)));
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
    boost::mutex::scoped_lock const l(mut);

    if(wq == weakQueue)
    {
      queue.reset();
      weakQueue.reset();
    }
  }
}

void TileViewState::computeBase(const ThreadPool::WeakQueue::Ptr& wq,
                                const ConstTile::Ptr&             tile_,
                                const LayerOperations::Ptr&       lo_)
{
  Scroom::Utils::Stuff const baseCache_ = lo_->cache(tile_);

  boost::mutex::scoped_lock const l(mut);
  TiledBitmapViewData::Ptr const  tbvd_ = tbvd.lock();

  if(tbvd_ && desiredState >= BASE_COMPUTED && weakQueue == wq)
  {
    baseCache = baseCache_;
    state     = BASE_COMPUTED;
  }
}

void TileViewState::computeZoom(const ThreadPool::WeakQueue::Ptr& wq,
                                const ConstTile::Ptr&             tile_,
                                const LayerOperations::Ptr&       lo_,
                                Scroom::Utils::Stuff              baseCache_,
                                int                               zoom_)
{
  Scroom::Utils::Stuff const zoomCache_ = lo_->cacheZoom(tile_, zoom_, baseCache_);

  boost::mutex::scoped_lock const l(mut);
  TiledBitmapViewData::Ptr const  tbvd_ = tbvd.lock();
  if(tbvd_ && desiredState >= ZOOM_COMPUTED && zoom == zoom_ && weakQueue == wq)
  {
    zoomCache = zoomCache_;
    state     = ZOOM_COMPUTED;
  }
}

void TileViewState::reportDone(const ThreadPool::WeakQueue::Ptr& /*wq*/, const ConstTile::Ptr& tile_)
{
  for(const TileLoadingObserver::Ptr& observer: Scroom::Utils::Observable<TileLoadingObserver>::getObservers())
  {
    observer->tileLoaded(tile_);
  }
}

void TileViewState::clear()
{
  boost::mutex::scoped_lock const l(mut);

  if(state >= LOADED)
  {
    state = LOADED;
  }

  queue.reset();
  weakQueue.reset();
  lifeTimeManager.reset();
  baseCache.reset();
  zoomCache.reset();

  kick();
}
