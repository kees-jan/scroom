/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

#ifndef _TILEINTERNAL_HH
#define _TILEINTERNAL_HH

#include <vector>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <scroom/tiledbitmapinterface.hh>
#include <scroom/tile.hh>

#include <scroom/observable.hh>

#include <scroom/memorymanagerinterface.hh>


#define TILESIZE 4096
// #define TILESIZE 1024

class TileInternal;

/** Observe a TileInternal object */
class TileInternalObserver
{
public:
  typedef boost::shared_ptr<TileInternalObserver> Ptr;
  typedef boost::weak_ptr<TileInternalObserver> WeakPtr;
  
  virtual ~TileInternalObserver() {}

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

/**
 * Internal data structure representing a Tile.
 *
 * The TileInternal class contains administrative information about
 * the Tile. The Tile itself (and associated bitmap data) will be
 * loaded and unloaded as needed. This class will stay in memory
 * always.
 *
 * Observers can receive events related to this tile.
 */
class TileInternal : public Scroom::Utils::Observable<TileInternalObserver>, public MemoryManagedInterface
{
public:
  typedef boost::shared_ptr<TileInternal> Ptr;
  
public:
  int depth;              /**< Layer number of this tile */
  int x;                  /**< x-coordinate of this tile (i.e. number of tiles to the left of this tile) */
  int y;                  /**< y-coordinate of this tile (i.e. number of tiles above this tile) */
  int bpp;                /**< Bits per pixel of this tile. Must be a divisor of 8. */
  TileState state;        /**< State of this tile */
  Tile::WeakPtr tile;     /**< Reference to the actual Tile */
  FileBackedMemory data;  /**< Data associated with the Tile */
  boost::mutex mut;       /**< Mutex protecting internal data structures */

private:
  TileInternal(int depth, int x, int y, int bpp, TileState state);

public:
  static Ptr create(int depth, int x, int y, int bpp, TileState state=TILE_UNINITIALIZED);

  /**
   * Initializes the tile data
   *
   * Allocates memory, initializes it to 0, and changes state to TILE_LOADED
   */
  void initialize();

  /**
   * Register an observer.
   *
   * Upon registering an observer, you'll receive the
   * TileInternalObserver::tileCreated() event immediately, on your
   * thread. Be careful with your mutexes :-)
   */
  Scroom::Utils::Registration registerObserver(TileInternalObserver::WeakPtr observer);

  /**
   * Register an observer.
   *
   * Upon registering an observer, you'll receive the
   * TileInternalObserver::tileCreated() event immediately, on your
   * thread. Be careful with your mutexes :-)
   */
  Scroom::Utils::Registration registerStrongObserver(TileInternalObserver::Ptr observer);

  /**
   * Get a reference to the Tile.
   *
   * If the tile is currently TILE_UNLOADED, it will be loaded. If the
   * tile is TILE_UNINITIALIZED, you'll receive an empty reference.
   */
  Tile::Ptr getTile();

  /**
   * Report that the tile is completely filled with data
   *
   * This'll be used to notify our observers.
   */
  void reportFinished();

  // MemoryManagedInterface //////////////////////////////////////////////
  virtual bool do_unload();
};

typedef std::vector<TileInternal::Ptr> TileInternalLine;
typedef std::vector<TileInternalLine> TileInternalGrid;

#endif
