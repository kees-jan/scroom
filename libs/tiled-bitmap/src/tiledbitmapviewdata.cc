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
#include "tiledbitmapviewdata.hh"

////////////////////////////////////////////////////////////////////////
// TiledBitmapViewData

TiledBitmapViewData::Ptr TiledBitmapViewData::create(ViewInterface* viewInterface)
{
  return TiledBitmapViewData::Ptr(new TiledBitmapViewData(viewInterface));
}

TiledBitmapViewData::TiledBitmapViewData(ViewInterface* viewInterface)
  : viewInterface(viewInterface), progressInterface(viewInterface->getProgressInterface()),
    layer(NULL), imin(0), imax(0), jmin(0), jmax(0), zoom(0), layerOperations(NULL)
{
}

TiledBitmapViewData::~TiledBitmapViewData()
{
  progressInterface->setState(ProgressInterface::IDLE);
}

void TiledBitmapViewData::setNeededTiles(Layer* l, int imin, int imax, int jmin, int jmax,
                                         int zoom, LayerOperations* layerOperations)
{
  boost::unique_lock<boost::mutex> lock(mut);

  if(this->layer == l && this->imin <= imin && this->imax >= imax &&
      this->jmin <= jmin && this->jmax >= jmax && this->zoom == zoom)
  {
    // Nothing to do...
  }
  else
  {
    this->layer = l;
    this->imin = imin;
    this->imax = imax;
    this->jmin = jmin;
    this->jmax = jmax;
    this->zoom = zoom;
    this->layerOperations = layerOperations;

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
      TileInternal::Ptr tile = layer->getTile(i,j);

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

gboolean invalidate_view(gpointer user_data)
{
  ViewInterface* vi = static_cast<ViewInterface*>(user_data);
  gdk_threads_enter();
  vi->invalidate();
  gdk_threads_leave();
  return false;
}

void TiledBitmapViewData::storeVolatileStuff(Scroom::Utils::Registration stuff)
{
  boost::unique_lock<boost::mutex> lock(mut);
  volatileStuff.push_back(stuff);
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

void TiledBitmapViewData::tileLoaded(Tile::Ptr tile)
{
  boost::unique_lock<boost::mutex> lock(mut);
  stuff.push_back(tile);

  // printf("TiledBitmapViewData::tileLoaded: redrawPending=%d\n", redrawPending);
  
  if(!redrawPending)
  {
    // We're not sure about whether gdk_threads_enter() has been
    // called or not, so we have no choice but to invalidate on
    // another thread.
    gtk_idle_add(invalidate_view, viewInterface);
    redrawPending = true;
  }
}

void TiledBitmapViewData::setState(State s)
{
  progressInterface->setState(s);
}

void TiledBitmapViewData::setProgress(double d)
{
  progressInterface->setProgress(d);
}

void TiledBitmapViewData::setProgress(int done, int total)
{
  progressInterface->setProgress(done, total);
}
