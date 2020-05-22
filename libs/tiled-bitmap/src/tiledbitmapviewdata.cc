/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "tiledbitmapviewdata.hh"

#include <scroom/gtk-helpers.hh>
#include <scroom/progressinterfacehelpers.hh>

#include "tileviewstate.hh"

////////////////////////////////////////////////////////////////////////
// TiledBitmapViewData

TiledBitmapViewData::Ptr TiledBitmapViewData::create(ViewInterface::WeakPtr viewInterface)
{
  return TiledBitmapViewData::Ptr(new TiledBitmapViewData(viewInterface));
}

TiledBitmapViewData::TiledBitmapViewData(ViewInterface::WeakPtr viewInterface_)
  : viewInterface(viewInterface_),
    progressInterface(viewInterface_.lock()->getProgressInterface()),
    layer(), imin(0), imax(0), jmin(0), jmax(0), zoom(0), layerOperations(),
    redrawPending(false)
{
}

void TiledBitmapViewData::setNeededTiles(Layer::Ptr const& l, int imin_, int imax_, int jmin_, int jmax_,
                                         int zoom_, LayerOperations::Ptr layerOperations_)
{
  boost::unique_lock<boost::mutex> lock(mut);

  if(this->layer == l && this->imin <= imin_ && this->imax >= imax_ &&
      this->jmin <= jmin_ && this->jmax >= jmax_ && this->zoom == zoom_)
  {
    // Nothing to do...
  }
  else
  {
    this->layer = l;
    this->imin = imin_;
    this->imax = imax_;
    this->jmin = jmin_;
    this->jmax = jmax_;
    this->zoom = zoom_;
    this->layerOperations = layerOperations_;

    // printf("SetNeededTiles: layer=%d, %d<=i<=%d, %d<=j<=%d, zoom=%d\n",
    //        l->getDepth(), imin, imax, jmin, jmax, zoom);

    // Get data for new tiles
    redrawPending = true; // if it isn't already
    // printf("setNeededTiles: redrawPending\n");
    lock.unlock();
    resetNeededTiles();
    lock.lock();
  }
  redrawPending = false;
  // printf("setNeededTiles: !redrawPending\n");
}

void TiledBitmapViewData::resetNeededTiles()
{
  boost::unique_lock<boost::mutex> lock(mut);

  // If we just cleared out oldStuff, old tiles would be unloaded, and
  // we might still need them. So create a backup to throw away later
  std::list<boost::shared_ptr<void> > oldStuff;
  oldStuff.swap(stuff);
  std::list<boost::shared_ptr<void> > oldVolatileStuff;
  oldVolatileStuff.swap(volatileStuff);

  lock.unlock();
  // The stuff list contains both registrations and references to needed tiles.
  // Registering an observer can result in tileLoaded() being called immediately
  // if the tile was already loaded. Hence, we cannot hold the lock while registering
  // observers.

  std::list<boost::shared_ptr<void> > newStuff;
  // If we don't hold the lock, we cannot add to the stuff list directly. Hence,
  // temporarily add registrations to the newStuff list, and add the newStuff to
  // stuff later.

  for(int i=imin; i<imax; i++)
    for(int j=jmin; j<jmax; j++)
    {
      CompressedTile::Ptr tile = layer->getTile(i,j);

      TileViewState::Ptr tileViewState = tile->getViewState(viewInterface);
      tileViewState->setViewData(shared_from_this<TiledBitmapViewData>());
      tileViewState->setZoom(layerOperations, zoom);
      newStuff.push_back(tileViewState);
      newStuff.push_back(tileViewState->registerObserver(shared_from_this<TiledBitmapViewData>()));
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

static gboolean invalidate_view(ViewInterface::WeakPtr vi)
{
  ViewInterface::Ptr v = vi.lock();
  if(v)
  {
    gdk_threads_enter();
    v->invalidate();
    gdk_threads_leave();
  }
  return false;
}

void TiledBitmapViewData::storeVolatileStuff(Scroom::Utils::Stuff stuff_)
{
  boost::unique_lock<boost::mutex> lock(mut);
  volatileStuff.push_back(stuff_);
}

void TiledBitmapViewData::clearVolatileStuff()
{
  //printf("Erasing volatile stuff for %p\n", viewInterface);
  std::list<boost::shared_ptr<void> > oldVolatileStuff;
  {
    boost::unique_lock<boost::mutex> lock(mut);
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
  boost::unique_lock<boost::mutex> lock(mut);
  stuff.push_back(tile);

  // printf("TiledBitmapViewData::tileLoaded: redrawPending=%d\n", redrawPending);

  if(!redrawPending)
  {
    // We're not sure about whether gdk_threads_enter() has been
    // called or not, so we have no choice but to invalidate on
    // another thread.
    Scroom::GtkHelpers::Wrapper w =
        Scroom::GtkHelpers::wrap(boost::bind(invalidate_view, viewInterface));
    gtk_idle_add(w.f, w.data);
    redrawPending = true;
  }
}

void TiledBitmapViewData::setIdle()
{
  progressInterface->setIdle();
}

void TiledBitmapViewData::setWaiting(double progress)
{
  progressInterface->setWaiting(progress);
}

void TiledBitmapViewData::setWorking(double progress)
{
  progressInterface->setWorking(progress);
}

void TiledBitmapViewData::setFinished()
{
  progressInterface->setFinished();
}
