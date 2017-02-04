/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
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
