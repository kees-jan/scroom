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

#ifndef TILEVIEWSTATE_HH
#define TILEVIEWSTATE_HH

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/thread.hpp>

#include <scroom/observable.hh>
#include <scroom/threadpool.hh>
#include <scroom/registration.hh>
#include <scroom/tiledbitmapinterface.hh>

#include "tileinternalobserverinterfaces.hh"

class TileInternal;
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
  boost::shared_ptr<TileInternal> parent;  
  boost::mutex mut;
  State state;
  State desiredState;
  ThreadPool::Queue::WeakPtr queue;
  ThreadPool::WeakQueue::WeakPtr weakQueue;
  Scroom::Utils::Registration r;
  Tile::Ptr tile;
  boost::weak_ptr<TiledBitmapViewData> tbvd;
  LayerOperations* lo;
  int zoom;
  Scroom::Utils::RegistrationWeak lifeTimeManager;
  Scroom::Utils::RegistrationWeak baseCache;
  Scroom::Utils::RegistrationWeak zoomCache;
  ThreadPool::Ptr cpuBound;
  
public:
  ~TileViewState();
  
  static Ptr create(boost::shared_ptr<TileInternal> parent);

  Scroom::Utils::Registration getCacheResult();
  void setViewData(boost::shared_ptr<TiledBitmapViewData> tbvd);
  void setZoom(LayerOperations* lo, int zoom);

  // TileLoadingObserver /////////////////////////////////////////////////
  virtual void tileLoaded(Tile::Ptr tile);

private:
  TileViewState(boost::shared_ptr<TileInternal> parent);

  /**
   * Kick the internal state machine into making some progress
   */
  void kick();

  /**
   * Asynchronously do work to make the state machine progress
   */
  void process(ThreadPool::WeakQueue::Ptr wq);

  void computeBase(ThreadPool::WeakQueue::Ptr wq, Tile::Ptr tile, LayerOperations* lo);
  void computeZoom(ThreadPool::WeakQueue::Ptr wq, Tile::Ptr tile, LayerOperations* lo,
                   Scroom::Utils::Registration baseCache, int zoom);
  void reportDone(ThreadPool::WeakQueue::Ptr wq, Tile::Ptr tile);
  void clear();
};


#endif
