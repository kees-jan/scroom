/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <functional>
#include <memory>

#include <scroom/rectangle.hh>
#include <scroom/tile-storage.hh>

class LayerInterface
  : public Viewable
  , public virtual Scroom::Utils::Base
{
public:
  using Ptr = std::shared_ptr<LayerInterface>;

  /** Valid image bounds, distinct from the allocated tile coverage. */
  virtual Scroom::Utils::Rectangle<int> getRect() const = 0;

  /** Allocated tile indices in the shared TILESIZE grid. */
  virtual Scroom::Utils::Rectangle<int> getTileRange() const = 0;

  virtual int getBpp() const = 0;

  /**
   * Indices outside getTileRange() return getOutOfBoundTile().
   */
  virtual TileStorage::Ptr getTile(Scroom::Utils::Point<int> tileIndex) = 0;

  /** Returns the shared tile whose state is TILE_OUT_OF_BOUNDS. */
  virtual TileStorage::Ptr getOutOfBoundTile() = 0;

  virtual Scroom::Bookkeeping::Token registerObserver(const TileInitialisationObserver::Ptr& observer) = 0;

  void open(ViewInterface::WeakPtr view) override = 0;
  void close(ViewInterface::WeakPtr view) override = 0;
};

/**
 * @param level Pyramid level: 0 is the bottom layer, 1 is the first reduced layer.
 * @param imageArea Valid image bounds in this level's pixel coordinates.
 * @param bpp Total bits per pixel, including all channels.
 */
using LayerFactory = std::function<LayerInterface::Ptr(int level, Scroom::Utils::Rectangle<int> imageArea, int bpp)>;
