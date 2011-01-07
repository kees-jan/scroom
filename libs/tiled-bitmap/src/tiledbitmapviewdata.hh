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

#ifndef _TILEDBITMAPVIEWDATA_HH
#define _TILEDBITMAPVIEWDATA_HH

#include <gtk/gtk.h>

#include <boost/shared_ptr.hpp>

#include <scroom/viewinterface.hh>

#include "layer.hh"

class TiledBitmapViewData
{
public:
  typedef boost::shared_ptr<TiledBitmapViewData> Ptr;

public:
  ViewInterface* viewInterface;
  GtkProgressBar* progressBar;

private:
  Layer* layer;
  int imin;
  int imax;
  int jmin;
  int jmax;

private:
  TiledBitmapViewData(ViewInterface* viewInterface);

public:
  static Ptr create(ViewInterface* viewInterface);
  virtual ~TiledBitmapViewData();

  void gtk_progress_bar_set_fraction(double fraction);
  void gtk_progress_bar_pulse();

  void setNeededTiles(Layer* l, int imin, int imax, int jmin, int jmax);
};

#endif
