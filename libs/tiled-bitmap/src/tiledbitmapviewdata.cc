/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "tiledbitmapviewdata.hh"

#include <utility>

#include <scroom/gtk-helpers.hh>
#include <scroom/progressinterfacehelpers.hh>

#include "tileviewstate.hh"

////////////////////////////////////////////////////////////////////////
// TiledBitmapViewData

TiledBitmapViewData::Ptr TiledBitmapViewData::create(const ViewInterface::WeakPtr& viewInterface)
{
  return TiledBitmapViewData::Ptr(new TiledBitmapViewData(std::move(viewInterface)));
}

TiledBitmapViewData::TiledBitmapViewData(const ViewInterface::WeakPtr& viewInterface_)
  : viewInterface(viewInterface_)
  , progressInterface(viewInterface_.lock()->getProgressInterface())

{
}

void TiledBitmapViewData::setNeededTiles(Layer::Ptr const&    l,
                                         int                  imin_,
                                         int                  imax_,
                                         int                  jmin_,
                                         int                  jmax_,
                                         int                  zoom_,
                                         LayerOperations::Ptr layerOperations_)
{
  boost::unique_lock<boost::mutex> lock(mut);

  if(layer == l && imin <= imin_ && imax >= imax_ && jmin <= jmin_ && jmax >= jmax_ && zoom == zoom_)
  {
    // Nothing to do...
  }
  else
  {
    layer           = l;
    imin            = imin_;
    imax            = imax_;
    jmin            = jmin_;
    jmax            = jmax_;
    zoom            = zoom_;
    layerOperations = std::move(layerOperations_);

    // Get data for new tiles
    redrawPending = true; // if it isn't already
    lock.unlock();
    resetNeededTiles();
    lock.lock();
  }
  redrawPending = false;
}

void TiledBitmapViewData::resetNeededTiles()
{
  boost::unique_lock<boost::mutex> lock(mut);

  // If we just cleared out oldStuff, old tiles would be unloaded, and
  // we might still need them. So create a backup to throw away later
  std::list<boost::shared_ptr<void>> oldStuff;
  oldStuff.swap(stuff);
  std::list<boost::shared_ptr<void>> oldVolatileStuff;
  oldVolatileStuff.swap(volatileStuff);

  lock.unlock();
  // The stuff list contains both registrations and references to needed tiles.
  // Registering an observer can result in tileLoaded() being called immediately
  // if the tile was already loaded. Hence, we cannot hold the lock while registering
  // observers.

  std::list<boost::shared_ptr<void>> newStuff;
  // If we don't hold the lock, we cannot add to the stuff list directly. Hence,
  // temporarily add registrations to the newStuff list, and add the newStuff to
  // stuff later.

  for(int i = imin; i < imax; i++)
  {
    for(int j = jmin; j < jmax; j++)
    {
      CompressedTile::Ptr const tile = layer->getTile(i, j);

      TileViewState::Ptr const tileViewState = tile->getViewState(viewInterface);
      tileViewState->setViewData(shared_from_this<TiledBitmapViewData>());
      tileViewState->setZoom(layerOperations, zoom);
      newStuff.emplace_back(tileViewState);
      newStuff.emplace_back(tileViewState->registerObserver(shared_from_this<TiledBitmapViewData>()));
    }
  }

  // At this point, everything we need is either in the stuff list, or the newStuff list.
  // Hence, this is an excellent time to clear the oldStuff list. We cannot clear the
  // oldStuff list while holding the lock, because that would result in deadlock (see
  // ticket #35)
  oldStuff.clear();
  oldVolatileStuff.clear();

  // Re-acquire the lock
  lock.lock();
  stuff.splice(stuff.end(), newStuff, newStuff.begin(), newStuff.end());
}

static void invalidate_view(const ViewInterface::WeakPtr& vi)
{
  ViewInterface::Ptr const v = vi.lock();
  if(v)
  {
    Scroom::GtkHelpers::sync_on_ui_thread([=] { v->invalidate(); });
  }
}

void TiledBitmapViewData::storeVolatileStuff(const Scroom::Utils::Stuff& stuff_)
{
  boost::unique_lock<boost::mutex> const lock(mut);
  volatileStuff.push_back(stuff_);
}

void TiledBitmapViewData::clearVolatileStuff()
{
  std::list<boost::shared_ptr<void>> oldVolatileStuff;
  {
    boost::unique_lock<boost::mutex> const lock(mut);
    oldVolatileStuff.swap(volatileStuff);
  }
  // oldVolatileStuff gets erased here, without holding the lock. So
  // if anyone is calling storeVolatileStuff() as a side effect, they
  // can do so safely.
  oldVolatileStuff.clear();
  resetNeededTiles();
}

void TiledBitmapViewData::tileLoaded(ConstTile::Ptr tile)
{
  boost::unique_lock<boost::mutex> const lock(mut);
  stuff.emplace_back(tile);

  if(!redrawPending)
  {
    // We're not sure about whether gdk_threads_enter() has been
    // called or not, so we have no choice but to invalidate on
    // another thread.
    Scroom::GtkHelpers::async_on_ui_thread([=] { invalidate_view(viewInterface); });
    redrawPending = true;
  }
}

void TiledBitmapViewData::setIdle() { progressInterface->setIdle(); }

void TiledBitmapViewData::setWaiting(double progress) { progressInterface->setWaiting(progress); }

void TiledBitmapViewData::setWorking(double progress) { progressInterface->setWorking(progress); }

void TiledBitmapViewData::setFinished() { progressInterface->setFinished(); }
