/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <map>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/interface.hh>
#include <scroom/memoryblobs.hh>
#include <scroom/observable.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/rectangle.hh>
#include <scroom/stuff.hh>
#include <scroom/threadpool.hh>
#include <scroom/tile.hh>
#include <scroom/tiledbitmapinterface.hh>

#define TILESIZE 4096
// #define TILESIZE 1024

/**
 * Represent the state of one of the tiles that make up a layer in the
 * bitmap.
 */
enum TileStateInternal
{
  TSI_UNINITIALIZED,
  TSI_NORMAL,
  TSI_OUT_OF_BOUNDS,
  TSI_LOADING_SYNCHRONOUSLY,
  TSI_LOADING_ASYNCHRONOUSLY
};

class TileViewState;
class CompressedTile;

////////////////////////////////////////////////////////////////////////

/** Events related to filling a tile with data. */
class TileInitialisationObserver : private Interface
{
public:
  using Ptr     = boost::shared_ptr<TileInitialisationObserver>;
  using WeakPtr = boost::weak_ptr<TileInitialisationObserver>;

  /**
   * The tile has been created.
   *
   * This event will be sent as soon as the observer is registered
   * (because obvously the tile has already been created beforehand).
   *
   * @note This event will be sent using the thread that is
   *    registering the observer. Be careful with your mutexes :-)
   */
  virtual void tileCreated(boost::shared_ptr<CompressedTile> tile);

  /**
   * This event will be sent when the tile is completely filled with
   * data. This would be a good time to update progress bars and start
   * prescaling.
   *
   * @note This event will be sent on the thread that is filling the
   *    tile with data.
   */
  virtual void tileFinished(boost::shared_ptr<CompressedTile> tile);
};

////////////////////////////////////////////////////////////////////////

/** Events related to swapping tiles in/out */
class TileLoadingObserver : private Interface
{
public:
  using Ptr     = boost::shared_ptr<TileLoadingObserver>;
  using WeakPtr = boost::weak_ptr<TileLoadingObserver>;

  /** The Tile has been loaded. */
  virtual void tileLoaded(ConstTile::Ptr tile) = 0;
};

////////////////////////////////////////////////////////////////////////

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
class CompressedTile
  : public Scroom::Utils::Observable<TileInitialisationObserver>
  , public Scroom::Utils::Observable<TileLoadingObserver>
  , public Viewable
{
public:
  using Ptr = boost::shared_ptr<CompressedTile>;

public:
  const int depth; /**< Layer number of this tile */
  const int x;     /**< x-coordinate of this tile (i.e. number of tiles to the left of this tile) */
  const int y;     /**< y-coordinate of this tile (i.e. number of tiles above this tile) */
  const int bpp;   /**< Bits per pixel of this tile. Must be a divisor of 8. */

private:
  TileStateInternal                      state;     /**< State of this tile */
  Tile::WeakPtr                          tile;      /**< Reference to the actual Tile */
  ConstTile::WeakPtr                     constTile; /**< Reference to the actual Tile */
  Scroom::MemoryBlobs::PageProvider::Ptr provider;  /**< Provider of blocks of memory */
  Scroom::MemoryBlobs::Blob::Ptr         data;      /**< Data associated with the Tile */
  boost::mutex                           stateData; /**< Mutex protecting the state field */
  boost::mutex                           tileData;  /**< Mutex protecting the data-related fields */

  ThreadPool::Queue::WeakPtr queue; /**< Queue on which the load operation is executed */

  std::map<ViewInterface::WeakPtr, boost::weak_ptr<TileViewState>> viewStates;

private:
  CompressedTile(int depth, int x, int y, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider, TileStateInternal state);

public:
  static Ptr create(int                                    depth,
                    int                                    x,
                    int                                    y,
                    int                                    bpp,
                    Scroom::MemoryBlobs::PageProvider::Ptr provider,
                    TileStateInternal                      state = TSI_UNINITIALIZED);

  /**
   * Initializes the tile data
   *
   * Allocates memory, initializes it to 0, and changes state to TSI_LOADED
   */
  Tile::Ptr initialize();

protected:
  /**
   * Keep track of new TileInitialisationObserver registrations.
   *
   * Upon registering an observer, you'll receive the
   * TileInitialisationObserver::tileCreated() event immediately, on your
   * thread. Be careful with your mutexes :-)
   */
  void observerAdded(TileInitialisationObserver::Ptr const& observer, Scroom::Bookkeeping::Token const& token) override;

  /**
   * Keep track of new TileLoadingObserver registrations.
   */
  void observerAdded(TileLoadingObserver::Ptr const& observer, Scroom::Bookkeeping::Token const& token) override;

public:
  // To choose between overloaded functions, the compiler needs some extra convincing
  virtual Scroom::Utils::Stuff registerStrongObserver(TileInitialisationObserver::Ptr const& observer)
  {
    return Scroom::Utils::Observable<TileInitialisationObserver>::registerStrongObserver(observer);
  }
  virtual Scroom::Utils::Stuff registerObserver(TileInitialisationObserver::WeakPtr const& observer)
  {
    return Scroom::Utils::Observable<TileInitialisationObserver>::registerObserver(observer);
  }
  virtual Scroom::Utils::Stuff registerStrongObserver(TileLoadingObserver::Ptr const& observer)
  {
    return Scroom::Utils::Observable<TileLoadingObserver>::registerStrongObserver(observer);
  }
  virtual Scroom::Utils::Stuff registerObserver(TileLoadingObserver::WeakPtr const& observer)
  {
    return Scroom::Utils::Observable<TileLoadingObserver>::registerObserver(observer);
  }

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
   * This will be used to notify our observers.
   */
  void reportFinished();

  TileState getState();

  boost::shared_ptr<TileViewState> getViewState(ViewInterface::WeakPtr vi);

private:
  /**
   * Does some internal state maintenance.
   *
   * Call only while stateData is locked.
   */
  void           cleanupState();
  ConstTile::Ptr do_load();
  void           notifyObservers(ConstTile::Ptr tile);

  // Viewable ////////////////////////////////////////////////////////////
public:
  void open(ViewInterface::WeakPtr vi) override;
  void close(ViewInterface::WeakPtr vi) override;
};

using CompressedTileLine = std::vector<CompressedTile::Ptr>;
using CompressedTileGrid = std::vector<CompressedTileLine>;

////////////////////////////////////////////////////////////////////////

class Layer
  : public Viewable
  , public virtual Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<Layer>;

private:
  int                      depth;
  int                      width;
  int                      height;
  Scroom::Utils::StuffList registrations;
  int                      horTileCount;
  int                      verTileCount;
  CompressedTileGrid       tiles;
  CompressedTile::Ptr      outOfBounds;
  CompressedTileLine       lineOutOfBounds;

private:
  Layer(TileInitialisationObserver::Ptr        observer,
        int                                    depth,
        int                                    layerWidth,
        int                                    layerHeight,
        int                                    bpp,
        Scroom::MemoryBlobs::PageProvider::Ptr provider);

public:
  static Ptr create(TileInitialisationObserver::Ptr        observer,
                    int                                    depth,
                    int                                    layerWidth,
                    int                                    layerHeight,
                    int                                    bpp,
                    Scroom::MemoryBlobs::PageProvider::Ptr provider);
  int        getHorTileCount();
  int        getVerTileCount();

  CompressedTile::Ptr getTile(int i, int j);
  CompressedTileLine& getTileLine(int j);
  void                fetchData(SourcePresentation::Ptr sp, ThreadPool::WeakQueue::Ptr queue);

public:
  int getWidth() { return width; }

  int getHeight() { return height; }

  int getDepth() { return depth; }

  Scroom::Utils::Rectangle<int> getRect() { return Scroom::Utils::Rectangle<int>(0, 0, width, height); }

public:
  // Viewable ////////////////////////////////////////////////////////////
  void open(ViewInterface::WeakPtr vi) override;
  void close(ViewInterface::WeakPtr vi) override;
};
