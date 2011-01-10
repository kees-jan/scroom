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
  : viewInterface(viewInterface), progressBar(viewInterface->getProgressBar()),
    layer(NULL), imin(0), imax(0), jmin(0), jmax(0)
{
}

TiledBitmapViewData::~TiledBitmapViewData()
{
  ::gtk_progress_bar_set_fraction(progressBar, 0.0);
}

void TiledBitmapViewData::gtk_progress_bar_set_fraction(double fraction)
{
  ::gtk_progress_bar_set_fraction(progressBar, fraction);
}

void TiledBitmapViewData::gtk_progress_bar_pulse()
{
  ::gtk_progress_bar_pulse(progressBar);
}

void TiledBitmapViewData::setNeededTiles(Layer* l, int imin, int imax, int jmin, int jmax)
{
  boost::unique_lock<boost::mutex> lock(mut);

  if(this->layer == l && this->imin <= imin && this->imax >= imax &&
      this->jmin <= jmin && this->jmax >= jmax)
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

    // Get data for new tiles
    redrawPending = true; // if it isn't already

    // If we just cleared out oldStuff, old tiles would be unloaded, and
    // we might still need them. So create a backup to throw away later
    std::list<boost::shared_ptr<void> > oldStuff;
    oldStuff.swap(stuff);

    lock.unlock();
    // Temporarily release the lock, such that tiles that have already been
    // loaded can be added to stuff

    // Without having the lock we can't add to stuff, so cache stuff here.
    std::list<boost::shared_ptr<void> > newStuff;

    for(int i=imin; i<imax; i++)
      for(int j=jmin; j<jmax; j++)
      {
        TileInternal::Ptr tile = layer->getTile(i,j);

        newStuff.push_back(tile->registerObserver(shared_from_this<TiledBitmapViewData>()));
      }
    // Re-acquire the lock
    lock.lock();
    stuff.splice(stuff.end(), newStuff, newStuff.begin(), newStuff.end());
  }
  redrawPending = false;
}

gboolean invalidate_view(gpointer user_data)
{
  ViewInterface* vi = static_cast<ViewInterface*>(user_data);
  gdk_threads_enter();
  vi->invalidate();
  gdk_threads_leave();
  return false;
}

void TiledBitmapViewData::tileLoaded(Tile::Ptr tile)
{
  boost::unique_lock<boost::mutex> lock(mut);
  stuff.push_back(tile);

  if(!redrawPending)
  {
    // We're not sure about whether gdk_threads_enter() has been
    // called or not, so we have no choice but to invalidate on
    // another thread.
    gtk_idle_add(invalidate_view, viewInterface);
    redrawPending = true;
  }
}
