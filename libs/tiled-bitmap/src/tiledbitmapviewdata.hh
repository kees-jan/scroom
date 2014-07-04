/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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
#include <scroom/observable.hh>
#include <scroom/bookkeeping.hh>

#include "layer.hh"

class TiledBitmapViewData : virtual public Scroom::Utils::Base, public TileLoadingObserver, public ProgressInterface
{
public:
  typedef boost::shared_ptr<TiledBitmapViewData> Ptr;

public:
  ViewInterface::WeakPtr viewInterface;
  ProgressInterface::Ptr progressInterface;
  Scroom::Bookkeeping::Token token;
  
private:
  Layer* layer;
  int imin;
  int imax;
  int jmin;
  int jmax;
  int zoom;
  LayerOperations::Ptr layerOperations;

  /**
   * References to things we want to keep around.
   *
   * This includes
   * @li observer registrations
   * @li tiles that have been loaded
   */
  Scroom::Utils::StuffList stuff;

  /**
   * References to things the user should be able to throw away on request
   *
   * This includes
   * @li pre-drawn bitmaps to make redraws go faster
   */
  Scroom::Utils::StuffList volatileStuff;

  bool redrawPending;

  /** Protect @c stuff and @c redrawPending */
  boost::mutex mut;

private:
  TiledBitmapViewData(ViewInterface::WeakPtr viewInterface);

public:
  static Ptr create(ViewInterface::WeakPtr viewInterface);

  void setNeededTiles(Layer* l, int imin, int imax, int jmin, int jmax, int zoom, LayerOperations::Ptr layerOperations);
  void resetNeededTiles();
  void storeVolatileStuff(Scroom::Utils::Stuff stuff);
  void clearVolatileStuff();

  // TileLoadingObserver ////////////////////////////////////////////////
  virtual void tileLoaded(ConstTile::Ptr tile);

  // ProgressInterface ///////////////////////////////////////////////////
  virtual void setIdle();
  virtual void setWaiting(double progress=0.0);
  virtual void setWorking(double progress);
  virtual void setFinished();
};

#endif
