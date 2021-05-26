/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <gtk/gtk.h>

#include <scroom/bookkeeping.hh>
#include <scroom/observable.hh>
#include <scroom/tiledbitmaplayer.hh>
#include <scroom/viewinterface.hh>

class TiledBitmapViewData
  : virtual public Scroom::Utils::Base
  , public TileLoadingObserver
  , public ProgressInterface
{
public:
  using Ptr = boost::shared_ptr<TiledBitmapViewData>;

public:
  ViewInterface::WeakPtr     viewInterface;
  ProgressInterface::Ptr     progressInterface;
  Scroom::Bookkeeping::Token token;

private:
  Layer::Ptr           layer;
  int                  imin;
  int                  imax;
  int                  jmin;
  int                  jmax;
  int                  zoom;
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

  void
       setNeededTiles(Layer::Ptr const& l, int imin, int imax, int jmin, int jmax, int zoom, LayerOperations::Ptr layerOperations);
  void resetNeededTiles();
  void storeVolatileStuff(Scroom::Utils::Stuff stuff);
  void clearVolatileStuff();

  // TileLoadingObserver ////////////////////////////////////////////////
  void tileLoaded(ConstTile::Ptr tile) override;

  // ProgressInterface ///////////////////////////////////////////////////
  void setIdle() override;
  void setWaiting(double progress = 0.0) override;
  void setWorking(double progress) override;
  void setFinished() override;
};
