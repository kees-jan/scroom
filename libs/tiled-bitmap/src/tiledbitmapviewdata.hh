/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>

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
  using Ptr = std::shared_ptr<TiledBitmapViewData>;

public:
  ViewInterface::WeakPtr     viewInterface;
  ProgressInterface::Ptr     progressInterface;
  Scroom::Bookkeeping::Token token;

private:
  Layer::Ptr           layer;
  int                  imin{0};
  int                  imax{0};
  int                  jmin{0};
  int                  jmax{0};
  int                  zoom{0};
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

  bool redrawPending{false};

  /** Protect @c stuff and @c redrawPending */
  boost::mutex mut;

private:
  explicit TiledBitmapViewData(const ViewInterface::WeakPtr& viewInterface);

public:
  static Ptr create(const ViewInterface::WeakPtr& viewInterface);

  void
    setNeededTiles(Layer::Ptr const& l, int imin, int imax, int jmin, int jmax, int zoom, LayerOperations::Ptr layerOperations);
  void resetNeededTiles();
  void storeVolatileStuff(const Scroom::Utils::Stuff& stuff);
  void clearVolatileStuff();

  // TileLoadingObserver ////////////////////////////////////////////////
  void tileLoaded(ConstTile::Ptr tile) override;

  // ProgressInterface ///////////////////////////////////////////////////
  void setIdle() override;
  void setWaiting(double progress = 0.0) override;
  void setWorking(double progress) override;
  void setFinished() override;
};
