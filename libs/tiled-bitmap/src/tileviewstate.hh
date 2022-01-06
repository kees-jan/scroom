/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/weak_ptr.hpp>

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
  using Ptr     = boost::shared_ptr<TileViewState>;
  using WeakPtr = boost::weak_ptr<TileViewState>;

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
  boost::shared_ptr<CompressedTile>    parent;
  boost::mutex                         mut;
  State                                state;
  State                                desiredState;
  ThreadPool::Queue::Ptr               queue;
  ThreadPool::WeakQueue::Ptr           weakQueue;
  Scroom::Utils::Stuff                 r;
  ConstTile::Ptr                       tile;
  boost::weak_ptr<TiledBitmapViewData> tbvd;
  LayerOperations::Ptr                 lo;
  int                                  zoom;
  Scroom::Utils::StuffWeak             lifeTimeManager;
  Scroom::Utils::Stuff                 baseCache;
  Scroom::Utils::Stuff                 zoomCache;
  ThreadPool::Ptr                      cpuBound;

public:
  ~TileViewState() override;
  TileViewState(const TileViewState&) = delete;
  TileViewState(TileViewState&&)      = delete;
  TileViewState operator=(const TileViewState&) = delete;
  TileViewState operator=(TileViewState&&) = delete;

  static Ptr create(boost::shared_ptr<CompressedTile> parent);

  Scroom::Utils::Stuff getCacheResult();
  void                 setViewData(boost::shared_ptr<TiledBitmapViewData> tbvd);
  void                 setZoom(LayerOperations::Ptr lo, int zoom);

  // TileLoadingObserver /////////////////////////////////////////////////
  void tileLoaded(ConstTile::Ptr tile) override;

private:
  TileViewState(boost::shared_ptr<CompressedTile> parent);

  /**
   * Kick the internal state machine into making some progress
   */
  void kick();

  /**
   * Asynchronously do work to make the state machine progress
   */
  void process(ThreadPool::WeakQueue::Ptr wq);

  void computeBase(ThreadPool::WeakQueue::Ptr wq, ConstTile::Ptr tile, LayerOperations::Ptr lo);
  void computeZoom(ThreadPool::WeakQueue::Ptr wq,
                   ConstTile::Ptr             tile,
                   LayerOperations::Ptr       lo,
                   Scroom::Utils::Stuff       baseCache,
                   int                        zoom);
  void reportDone(ThreadPool::WeakQueue::Ptr wq, ConstTile::Ptr tile);
  void clear();
};
