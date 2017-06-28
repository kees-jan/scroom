/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

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
#include <scroom/memoryblobs.hh>
#include <scroom/stuff.hh>

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
    TSI_UNINITIALIZED,
    TSI_NORMAL,
    TSI_OUT_OF_BOUNDS,
    TSI_LOADING_SYNCHRONOUSLY,
    TSI_LOADING_ASYNCHRONOUSLY
  } TileStateInternal;


/**
 * Internal data structure representing a Tile.
 *
 * The CompressedTile class contains administrative information about
 * the Tile. The Tile itself (and associated bitmap data) will be
 * loaded and unloaded as needed. This class will stay in memory
 * always.
 *
 * Observers can receive events related to this tile.
 */
class CompressedTile : public Scroom::Utils::Observable<TileInitialisationObserver>,
                     public Scroom::Utils::Observable<TileLoadingObserver>,
                     public Viewable
{
public:
  typedef boost::shared_ptr<CompressedTile> Ptr;
  
public:
  int depth;              /**< Layer number of this tile */
  int x;                  /**< x-coordinate of this tile (i.e. number of tiles to the left of this tile) */
  int y;                  /**< y-coordinate of this tile (i.e. number of tiles above this tile) */
  int bpp;                /**< Bits per pixel of this tile. Must be a divisor of 8. */
  TileStateInternal state;/**< State of this tile */
  Tile::WeakPtr tile;     /**< Reference to the actual Tile */
  ConstTile::WeakPtr constTile;     /**< Reference to the actual Tile */
  Scroom::MemoryBlobs::PageProvider::Ptr provider;  /**< Provider of blocks of memory */
  Scroom::MemoryBlobs::Blob::Ptr data;              /**< Data associated with the Tile */
  boost::mutex stateData; /**< Mutex protecting the state field */
  boost::mutex tileData;  /**< Mutex protecting the data-related fields */

  ThreadPool::Queue::WeakPtr queue; /**< Queue on which the load operation is executed */

  std::map<ViewInterface::WeakPtr, TileViewState::WeakPtr> viewStates;

private:
  CompressedTile(int depth, int x, int y, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider, TileStateInternal state);

public:
  static Ptr create(int depth, int x, int y, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider, TileStateInternal state=TSI_UNINITIALIZED);

  /**
   * Initializes the tile data
   *
   * Allocates memory, initializes it to 0, and changes state to TSI_LOADED
   */
  Scroom::Utils::Stuff initialize();

protected:
  /**
   * Keep track of new TileInitialisationObserver registrations.
   *
   * Upon registering an observer, you'll receive the
   * TileInitialisationObserver::tileCreated() event immediately, on your
   * thread. Be careful with your mutexes :-)
   */
  virtual void observerAdded(TileInitialisationObserver::Ptr const& observer, Scroom::Bookkeeping::Token const& token);

  /**
   * Keep track of new TileLoadingObserver registrations.
   */
  virtual void observerAdded(TileLoadingObserver::Ptr const& observer, Scroom::Bookkeeping::Token const& token);

public:
  // To choose between overloaded functions, the compiler needs some extra convincing
  virtual Scroom::Utils::Stuff registerStrongObserver(TileInitialisationObserver::Ptr const& observer) { return Scroom::Utils::Observable<TileInitialisationObserver>::registerStrongObserver(observer); }
  virtual Scroom::Utils::Stuff registerObserver(TileInitialisationObserver::WeakPtr const& observer) { return Scroom::Utils::Observable<TileInitialisationObserver>::registerObserver(observer); }
  virtual Scroom::Utils::Stuff registerStrongObserver(TileLoadingObserver::Ptr const& observer) { return Scroom::Utils::Observable<TileLoadingObserver>::registerStrongObserver(observer); }
  virtual Scroom::Utils::Stuff registerObserver(TileLoadingObserver::WeakPtr const& observer) { return Scroom::Utils::Observable<TileLoadingObserver>::registerObserver(observer); }

  /**
   * Get a reference to the Tile.
   *
   * If the tile is currently TSI_NORMAL, it will be loaded, if necessary. If the
   * tile is TSI_UNINITIALIZED, you'll receive an empty reference.
   */
  Tile::Ptr getTileSync();

  /**
   * Get a reference to the Const Tile.
   *
   * If the tile is currently TSI_NORMAL, it will be loaded, if necessary. If the
   * tile is TSI_UNINITIALIZED, you'll receive an empty reference.
   */
  ConstTile::Ptr getConstTileSync();

  /**
   * Get a reference to the ConstTile.
   *
   * If the tile is currently TSI_LOADED, you'll get a reference, otherwise,
   * you'll receive an empty reference.
   */
  ConstTile::Ptr getConstTileAsync();

  /**
   * Report that the tile is completely filled with data
   *
   * This'll be used to notify our observers.
   */
  void reportFinished();

  TileState getState();

  TileViewState::Ptr getViewState(ViewInterface::WeakPtr vi);

private:
  /**
   * Does some internal state maintenance.
   *
   * Call only while stateData is locked.
   */
  void cleanupState();
  ConstTile::Ptr do_load();
  void notifyObservers(ConstTile::Ptr tile);

  // Viewable ////////////////////////////////////////////////////////////
public:
  
  virtual void open(ViewInterface::WeakPtr vi);
  virtual void close(ViewInterface::WeakPtr vi);
};

typedef std::vector<CompressedTile::Ptr> CompressedTileLine;
typedef std::vector<CompressedTileLine> CompressedTileGrid;


