/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <map>
#include <memory>
#include <vector>

#include <boost/thread.hpp>

#include <scroom/interface.hh>
#include <scroom/layer-base.hh>
#include <scroom/logger.hh>
#include <scroom/memoryblobs.hh>
#include <scroom/observable.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/rectangle.hh>
#include <scroom/stuff.hh>
#include <scroom/threadpool.hh>
#include <scroom/tile-storage.hh>
#include <scroom/tile.hh>
#include <scroom/tiled-bitmap-data.hh>


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
class CompressedTile : public TileStorage
{
public:
  using Ptr = std::shared_ptr<CompressedTile>;

public:
  const int depth; /**< Layer number of this tile */
  const int x; /**< x-coordinate of this tile (i.e. number of tiles to the left of this tile) */
  const int y; /**< y-coordinate of this tile (i.e. number of tiles above this tile) */
  const int bpp; /**< Bits per pixel of this tile. */

  int getBpp() const override { return bpp; }

private:
  TileStateInternal state; /**< State of this tile */
  Tile::WeakPtr tile; /**< Reference to the actual Tile */
  ConstTile::WeakPtr constTile; /**< Reference to the actual Tile */
  Scroom::MemoryBlobs::PageProvider::Ptr provider; /**< Provider of blocks of memory */
  Scroom::MemoryBlobs::Blob::Ptr data; /**< Data associated with the Tile */
  boost::mutex stateData; /**< Mutex protecting the state field */
  boost::mutex tileData; /**< Mutex protecting the data-related fields */

  ThreadPool::Queue::WeakPtr queue; /**< Queue on which the load operation is executed */


private:
  CompressedTile(
    int depth,
    int x,
    int y,
    int bpp,
    const Scroom::MemoryBlobs::PageProvider::Ptr& provider,
    TileStateInternal state
  );

public:
  static Ptr create(
    int depth,
    int x,
    int y,
    int bpp,
    const Scroom::MemoryBlobs::PageProvider::Ptr& provider,
    TileStateInternal state = TSI_UNINITIALIZED
  );

  Scroom::Utils::Stuff initialize() override;

protected:
  using TileStorage::observerAdded;

  /**
   * Keep track of new TileLoadingObserver registrations.
   */
  void observerAdded(TileLoadingObserver::Ptr const& observer, Scroom::Bookkeeping::Token const& token) override;

public:
  /**
   * Get a reference to the Tile.
   *
   * If the tile is currently TSI_NORMAL, it will be loaded, if necessary. If the
   * tile is TSI_UNINITIALIZED, you'll receive an empty reference.
   */
  Tile::Ptr getTileSync() override;

  /**
   * Get a reference to the Const Tile.
   *
   * If the tile is currently TSI_NORMAL, it will be loaded, if necessary. If the
   * tile is TSI_UNINITIALIZED, you'll receive an empty reference.
   */
  ConstTile::Ptr getConstTileSync() override;

  /**
   * Get a reference to the ConstTile.
   *
   * If the tile is currently TSI_LOADED, you'll get a reference, otherwise,
   * you'll receive an empty reference.
   */
  ConstTile::Ptr getConstTileAsync() override;

  /**
   * Report that the tile is completely filled with data
   *
   * This will be used to notify our observers.
   */
  void reportFinished() override;

  TileState getState() override;


private:
  /**
   * Does some internal state maintenance.
   *
   * Call only while stateData is locked.
   */
  void cleanupState();
  ConstTile::Ptr do_load();
  void notifyObservers(const ConstTile::Ptr& tile);
};

using CompressedTileLine = std::vector<CompressedTile::Ptr>;
using CompressedTileGrid = std::vector<CompressedTileLine>;

////////////////////////////////////////////////////////////////////////

/** Default layer backend using compressed tile storage. */
class Layer : public LayerBase
{
public:
  using Ptr = std::shared_ptr<Layer>;

private:
  int width;
  int height;
  int bitsPerPixel;
  int horTileCount;
  int verTileCount;
  CompressedTileGrid tiles;
  CompressedTile::Ptr outOfBounds;
  Scroom::MemoryBlobs::PageProvider::Ptr pageProvider;
  Scroom::Logger logger;

private:
  Layer(int depth, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider);

public:
  static Ptr create(int depth, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider);
  static Ptr create(int layerWidth, int layerHeight, int bpp);

  TileStorage::Ptr getTile(Scroom::Utils::Point<int> tileIndex) override;
  TileStorage::Ptr getOutOfBoundTile() override { return outOfBounds; }

public:
  int getBpp() const override { return bitsPerPixel; }

  Scroom::Utils::Rectangle<int> getTileRange() const override { return {0, 0, horTileCount, verTileCount}; }

  Scroom::Utils::Rectangle<int> getRect() const override { return {0, 0, width, height}; }


  Scroom::MemoryBlobs::PageProvider::Ptr getPageProvider() { return pageProvider; }
};

Scroom::MemoryBlobs::PageProvider::Ptr createCompressedLayerPageProvider(Scroom::Utils::Point<int> imageSize, int bpp);
LayerFactory createCompressedLayerFactory(Scroom::MemoryBlobs::PageProvider::Ptr provider);
