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

    // TODO: Get data for new tiles
  }
}
