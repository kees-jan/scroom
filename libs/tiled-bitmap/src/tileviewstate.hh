/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>

#include <boost/thread.hpp>

#include <scroom/observable.hh>
#include <scroom/stuff.hh>
#include <scroom/threadpool.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/tiledbitmaplayer.hh>

class TiledBitmapViewData;

class TileViewState
  : public Scroom::Utils::Observable<TileLoadingObserver>
  , public TileLoadingObserver
{
public:
  using Ptr     = std::shared_ptr<TileViewState>;
  using WeakPtr = std::weak_ptr<TileViewState>;

  enum State
  {
    INIT,
    LOADED,
    COMPUTING_BASE,
    BASE_COMPUTED,
    COMPUTING_ZOOM,
    ZOOM_COMPUTED,
    DONE
  };

private:
  std::shared_ptr<CompressedTile>    parent;
  boost::mutex                       mut;
  State                              state{INIT};
  State                              desiredState{LOADED};
  ThreadPool::Queue::Ptr             queue;
  ThreadPool::WeakQueue::Ptr         weakQueue;
  Scroom::Utils::Stuff               r;
  ConstTile::Ptr                     tile;
  std::weak_ptr<TiledBitmapViewData> tbvd;
  LayerOperations::Ptr               lo;
  int                                zoom{0};
  Scroom::Utils::StuffWeak           lifeTimeManager;
  Scroom::Utils::Stuff               baseCache;
  Scroom::Utils::Stuff               zoomCache;
  ThreadPool::Ptr                    cpuBound;

public:
  ~TileViewState() override;
  TileViewState(const TileViewState&)           = delete;
  TileViewState(TileViewState&&)                = delete;
  TileViewState operator=(const TileViewState&) = delete;
  TileViewState operator=(TileViewState&&)      = delete;

  static Ptr create(const std::shared_ptr<CompressedTile>& parent);

  Scroom::Utils::Stuff getCacheResult();
  void                 setViewData(const std::shared_ptr<TiledBitmapViewData>& tbvd);
  void                 setZoom(LayerOperations::Ptr lo, int zoom);

  // TileLoadingObserver /////////////////////////////////////////////////
  void tileLoaded(ConstTile::Ptr tile) override;

private:
  explicit TileViewState(std::shared_ptr<CompressedTile> parent);

  /**
   * Kick the internal state machine into making some progress
   */
  void kick();

  /**
   * Asynchronously do work to make the state machine progress
   */
  void process(const ThreadPool::WeakQueue::Ptr& wq);

  void computeBase(const ThreadPool::WeakQueue::Ptr& wq, const ConstTile::Ptr& tile_, const LayerOperations::Ptr& lo_);
  void computeZoom(const ThreadPool::WeakQueue::Ptr& wq,
                   const ConstTile::Ptr&             tile,
                   const LayerOperations::Ptr&       lo,
                   Scroom::Utils::Stuff              baseCache,
                   int                               zoom);
  void reportDone(const ThreadPool::WeakQueue::Ptr& wq, const ConstTile::Ptr& tile);
  void clear();
};
