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

#ifndef _TILEINTERNAL_HH
#define _TILEINTERNAL_HH

#include <vector>
#include <map>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <scroom/presentationinterface.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/tile.hh>

#include <scroom/observable.hh>
#include <scroom/threadpool.hh>
#include <scroom/memorymanagerinterface.hh>

#include "tileinternalobserverinterfaces.hh"
#include "tileviewstate.hh"


#define TILESIZE 4096
// #define TILESIZE 1024

/**
 * Represent the state of one of the tiles that make up a layer in the
 * bitmap.
 */
typedef enum
  {
    TSI_UNINITIALIZED = TILE_UNINITIALIZED, /**< Tile does not yet contain any data */
    TSI_UNLOADED = TILE_UNLOADED,           /**< Tile does contain data, but has been swapped out */
    TSI_LOADED = TILE_LOADED,               /**< Tile does contain data, and data is in memory */
    TSI_OUT_OF_BOUNDS = TILE_OUT_OF_BOUNDS, /**< Tile is located outside the bitmap area */
    TSI_LOADING_SYNCHRONOUSLY,
    TSI_LOADING_ASYNCHRONOUSLY
  } TileStateInternal;


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
class TileInternal : public Scroom::Utils::Observable<TileInitialisationObserver>,
                     public Scroom::Utils::Observable<TileLoadingObserver>,
                     public MemoryManagedInterface,
                     public Viewable
{
public:
  typedef boost::shared_ptr<TileInternal> Ptr;
  
public:
  int depth;              /**< Layer number of this tile */
  int x;                  /**< x-coordinate of this tile (i.e. number of tiles to the left of this tile) */
  int y;                  /**< y-coordinate of this tile (i.e. number of tiles above this tile) */
  int bpp;                /**< Bits per pixel of this tile. Must be a divisor of 8. */
  TileStateInternal state;/**< State of this tile */
  Tile::WeakPtr tile;     /**< Reference to the actual Tile */
  FileBackedMemory data;  /**< Data associated with the Tile */
  boost::mutex stateData; /**< Mutex protecting the state field */
  boost::mutex tileData;  /**< Mutex protecting the data-related fields */

  ThreadPool::Queue::WeakPtr queue; /**< Queue on which the load operation is executed */
  Scroom::Utils::Stuff memoryManagerRegistration; /**< Our registration with the MemoryManager */

  std::map<ViewInterface*, TileViewState::WeakPtr> viewStates;

private:
  TileInternal(int depth, int x, int y, int bpp, TileStateInternal state);

public:
  static Ptr create(int depth, int x, int y, int bpp, TileStateInternal state=TSI_UNINITIALIZED);

  /**
   * Initializes the tile data
   *
   * Allocates memory, initializes it to 0, and changes state to TSI_LOADED
   */
  void initialize();

  /**
   * Keep track of new TileInitialisationObserver registrations.
   *
   * Upon registering an observer, you'll receive the
   * TileInitialisationObserver::tileCreated() event immediately, on your
   * thread. Be careful with your mutexes :-)
   */
  virtual void observerAdded(TileInitialisationObserver::Ptr observer);

  /**
   * Keep track of new TileLoadingObserver registrations.
   */
  virtual void observerAdded(TileLoadingObserver::Ptr observer);

  // To choose between overloaded functions, the compiler needs some extra convincing
  virtual Scroom::Utils::Stuff registerStrongObserver(TileInitialisationObserver::Ptr observer) { return Scroom::Utils::Observable<TileInitialisationObserver>::registerStrongObserver(observer); }
  virtual Scroom::Utils::Stuff registerObserver(TileInitialisationObserver::WeakPtr observer) { return Scroom::Utils::Observable<TileInitialisationObserver>::registerObserver(observer); }
  virtual Scroom::Utils::Stuff registerStrongObserver(TileLoadingObserver::Ptr observer) { return Scroom::Utils::Observable<TileLoadingObserver>::registerStrongObserver(observer); }
  virtual Scroom::Utils::Stuff registerObserver(TileLoadingObserver::WeakPtr observer) { return Scroom::Utils::Observable<TileLoadingObserver>::registerObserver(observer); }

  /**
   * Get a reference to the Tile.
   *
   * If the tile is currently TSI_UNLOADED, it will be loaded. If the
   * tile is TSI_UNINITIALIZED, you'll receive an empty reference.
   */
  Tile::Ptr getTileSync();

  /**
   * Get a reference to the Tile.
   *
   * If the tile is currently TSI_LOADED, you'll get a reference, otherwise,
   * you'll receive an empty reference.
   */
  Tile::Ptr getTileAsync();

  /**
   * Report that the tile is completely filled with data
   *
   * This'll be used to notify our observers.
   */
  void reportFinished();

  TileState getState();

  TileViewState::Ptr getViewState(ViewInterface* vi);

  void performMemoryManagerRegistration();

private:
  /**
   * Does some internal state maintenance.
   *
   * Call only while stateData is locked.
   */
  void cleanupState();
  Tile::Ptr do_load();
  void notifyObservers(Tile::Ptr tile);

  // Viewable ////////////////////////////////////////////////////////////
public:
  
  virtual void open(ViewInterface* vi);
  virtual void close(ViewInterface* vi);
  
  // MemoryManagedInterface //////////////////////////////////////////////
public:
  virtual bool do_unload();
};

typedef std::vector<TileInternal::Ptr> TileInternalLine;
typedef std::vector<TileInternalLine> TileInternalGrid;

#endif
