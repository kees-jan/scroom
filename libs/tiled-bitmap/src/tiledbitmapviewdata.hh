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
#include <scroom/observable.hh>

#include "layer.hh"

class TiledBitmapViewData : public Scroom::Utils::Base, public TileLoadingObserver, public ProgressInterface
{
public:
  typedef boost::shared_ptr<TiledBitmapViewData> Ptr;

public:
  ViewInterface* viewInterface;
  ProgressInterface* progressInterface;

private:
  Layer* layer;
  int imin;
  int imax;
  int jmin;
  int jmax;
  int zoom;
  LayerOperations* layerOperations;

  /**
   * References to things we want to keep around.
   *
   * This includes
   * @li observer registrations
   * @li tiles that have been loaded
   */
  std::list<boost::shared_ptr<void> > stuff;

  /**
   * References to things the user should be able to throw away on request
   *
   * This includes
   * @li pre-drawn bitmaps to make redraws go faster
   */
  std::list<boost::shared_ptr<void> > volatileStuff;

  bool redrawPending;

  /** Protect @c stuff and @c redrawPending */
  boost::mutex mut;

private:
  TiledBitmapViewData(ViewInterface* viewInterface);

public:
  static Ptr create(ViewInterface* viewInterface);
  virtual ~TiledBitmapViewData();

  void setNeededTiles(Layer* l, int imin, int imax, int jmin, int jmax, int zoom, LayerOperations* layerOperations);
  void resetNeededTiles();
  void storeVolatileStuff(Scroom::Utils::Registration stuff);
  void clearVolatileStuff();

  // TileLoadingObserver ////////////////////////////////////////////////
  virtual void tileLoaded(Tile::Ptr tile);

  // ProgressInterface ///////////////////////////////////////////////////
  virtual void setState(State s);
  virtual void setProgress(double d);
  virtual void setProgress(int done, int total);
  
};

#endif
