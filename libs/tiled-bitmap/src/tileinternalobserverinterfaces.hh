/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

#ifndef TILEINTERNALOBSERVERINTERFACES_HH
#define TILEINTERNALOBSERVERINTERFACES_HH

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/tile.hh>

class TileInternal;

/** Events related to filling a tile with data. */
class TileInitialisationObserver
{
public:
  typedef boost::shared_ptr<TileInitialisationObserver> Ptr;
  typedef boost::weak_ptr<TileInitialisationObserver> WeakPtr;
  
  virtual ~TileInitialisationObserver() {}

  /**
   * The tile has been created.
   *
   * This event will be sent as soon as the observer is registered
   * (because obvously the tile has already been created beforehand.
   *
   * @note This event will be sent using the thread that is
   *    registering the observer. Be careful with your mutexes :-)
   */
  virtual void tileCreated(boost::shared_ptr<TileInternal> tile);

  /**
   * This event will be sent when the tile is completely filled with
   * data. This would be a good time to update progress bars and start
   * prescaling.
   *
   * @note This event will be sent on the thread that is filling the
   *    tile with data.
   */ 
  virtual void tileFinished(boost::shared_ptr<TileInternal> tile);
};

/** Events related to swapping tiles in/out */
class TileLoadingObserver
{
public:
  typedef boost::shared_ptr<TileLoadingObserver> Ptr;
  typedef boost::weak_ptr<TileLoadingObserver> WeakPtr;

  virtual ~TileLoadingObserver() {};

  /** The Tile has been loaded. */
  virtual void tileLoaded(ConstTile::Ptr tile)=0;
};

#endif
