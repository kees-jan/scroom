/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>

#include <scroom/bookkeeping.hh>
#include <scroom/observable.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/tile.hh>

class TileStorage;
class TileViewState;

/** Events related to filling a tile with data. */
class TileInitialisationObserver : private Interface
{
public:
  using Ptr = std::shared_ptr<TileInitialisationObserver>;
  using WeakPtr = std::weak_ptr<TileInitialisationObserver>;

  /**
   * The tile has been created.
   *
   * This event will be sent as soon as the observer is registered
   * (because obvously the tile has already been created beforehand).
   *
   * @note This event will be sent using the thread that is
   *    registering the observer. Be careful with your mutexes :-)
   */
  virtual void tileCreated(const std::shared_ptr<TileStorage>& tile);

  /**
   * This event will be sent when the tile is completely filled with
   * data. This would be a good time to update progress bars and start
   * prescaling.
   *
   * @note This event will be sent on the thread that is filling the
   *    tile with data.
   */
  virtual void tileFinished(const std::shared_ptr<TileStorage>& tile);
};

////////////////////////////////////////////////////////////////////////

/** Events related to swapping tiles in/out */
class TileLoadingObserver : private Interface
{
public:
  using Ptr = std::shared_ptr<TileLoadingObserver>;
  using WeakPtr = std::weak_ptr<TileLoadingObserver>;

  /** The Tile has been loaded. */
  virtual void tileLoaded(ConstTile::Ptr tile) = 0;
};

////////////////////////////////////////////////////////////////////////

class TileStorage
  : public Scroom::Utils::Observable<TileInitialisationObserver>
  , public Scroom::Utils::Observable<TileLoadingObserver>
  , public Viewable
{
public:
  using Ptr = std::shared_ptr<TileStorage>;

  virtual int getBpp() const = 0;

  /**
   * Safe to call repeatedly and concurrently; existing pixels are preserved.
   * The returned object may keep pixel data in memory so subsequent access
   * avoids reloading it.
   */
  virtual Scroom::Utils::Stuff initialize() = 0;

  virtual Tile::Ptr getTileSync() = 0;

  virtual ConstTile::Ptr getConstTileSync() = 0;

  /** Returns nullptr if pixels are not currently in memory; does not load them. */
  virtual ConstTile::Ptr getConstTileAsync() = 0;

  virtual TileState getState() = 0;
  virtual void reportFinished();

  // To choose between overloaded functions, the compiler needs some extra convincing
  Scroom::Bookkeeping::Token registerStrongObserver(TileInitialisationObserver::Ptr const& observer)
  {
    return Scroom::Utils::Observable<TileInitialisationObserver>::registerStrongObserver(observer);
  }

  Scroom::Bookkeeping::Token registerObserver(TileInitialisationObserver::WeakPtr const& observer)
  {
    return Scroom::Utils::Observable<TileInitialisationObserver>::registerObserver(observer);
  }

  Scroom::Bookkeeping::Token registerStrongObserver(TileLoadingObserver::Ptr const& observer)
  {
    return Scroom::Utils::Observable<TileLoadingObserver>::registerStrongObserver(observer);
  }

  Scroom::Bookkeeping::Token registerObserver(TileLoadingObserver::WeakPtr const& observer)
  {
    return Scroom::Utils::Observable<TileLoadingObserver>::registerObserver(observer);
  }

  std::shared_ptr<TileViewState> getViewState(const ViewInterface::WeakPtr& view);
  void open(ViewInterface::WeakPtr view) override;
  void close(ViewInterface::WeakPtr view) override;

protected:
  void observerAdded(const TileInitialisationObserver::Ptr& observer, const Scroom::Bookkeeping::Token& token) override;
  void observerAdded(const TileLoadingObserver::Ptr& observer, const Scroom::Bookkeeping::Token& token) override;

private:
  Scroom::Utils::WeakKeyMap<ViewInterface::WeakPtr, std::weak_ptr<TileViewState>> m_viewStates;
};
