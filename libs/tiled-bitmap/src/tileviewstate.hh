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

#include "tileinternalobserverinterfaces.hh"

class TileInternal;

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
  ThreadPool::Queue::Ptr queue;
  Scroom::Utils::Registration r;
  Tile::Ptr tile;
  
public:
  ~TileViewState();
  
  static Ptr create(boost::shared_ptr<TileInternal> parent);

  // TileLoadingObserver /////////////////////////////////////////////////
  virtual void tileLoaded(Tile::Ptr tile);

private:
  TileViewState(boost::shared_ptr<TileInternal> parent);
  
  
};


#endif
