/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread.hpp>

#include <scroom/observable.hh>
#include <scroom/threadpool.hh>
#include <scroom/stuff.hh>
#include <scroom/tiledbitmapinterface.hh>

#include "tileinternalobserverinterfaces.hh"

class CompressedTile;
class TiledBitmapViewData;

class TileViewState : public Scroom::Utils::Observable<TileLoadingObserver>,
                      public TileLoadingObserver
{
public:
  typedef boost::shared_ptr<TileViewState> Ptr;
  typedef boost::weak_ptr<TileViewState> WeakPtr;

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
  boost::shared_ptr<CompressedTile> parent;  
  boost::mutex mut;
  State state;
  State desiredState;
  ThreadPool::Queue::Ptr queue;
  ThreadPool::WeakQueue::Ptr weakQueue;
  Scroom::Utils::Stuff r;
  ConstTile::Ptr tile;
  boost::weak_ptr<TiledBitmapViewData> tbvd;
  LayerOperations::Ptr lo;
  int zoom;
  Scroom::Utils::StuffWeak lifeTimeManager;
  Scroom::Utils::Stuff baseCache;
  Scroom::Utils::Stuff zoomCache;
  ThreadPool::Ptr cpuBound;
  
public:
  ~TileViewState();
  
  static Ptr create(boost::shared_ptr<CompressedTile> parent);

  Scroom::Utils::Stuff getCacheResult();
  void setViewData(boost::shared_ptr<TiledBitmapViewData> tbvd);
  void setZoom(LayerOperations::Ptr lo, int zoom);

  // TileLoadingObserver /////////////////////////////////////////////////
  virtual void tileLoaded(ConstTile::Ptr tile);

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
  void computeZoom(ThreadPool::WeakQueue::Ptr wq, ConstTile::Ptr tile, LayerOperations::Ptr lo,
                   Scroom::Utils::Stuff baseCache, int zoom);
  void reportDone(ThreadPool::WeakQueue::Ptr wq, ConstTile::Ptr tile);
  void clear();
};



