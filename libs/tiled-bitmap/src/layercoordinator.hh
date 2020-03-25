/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <map>
#include <utility>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "scroom/tiledbitmaplayer.hh"

class LayerCoordinator: public TileInitialisationObserver,
                        public virtual Scroom::Utils::Base
{
private:
  CompressedTile::Ptr targetTile;
  Tile::Ptr targetTileData;
  std::map<CompressedTile::Ptr,std::pair<int,int> > sourceTiles;
  Scroom::Utils::StuffList registrations;
  LayerOperations::Ptr lo;
  boost::mutex mut;
  int unfinishedSourceTiles;

public:
  typedef boost::shared_ptr<LayerCoordinator> Ptr;

  static Ptr create(CompressedTile::Ptr targetTile, LayerOperations::Ptr lo);

  virtual ~LayerCoordinator();

  void addSourceTile(int x, int y, CompressedTile::Ptr tile);

private:
  LayerCoordinator(CompressedTile::Ptr targetTile, LayerOperations::Ptr lo);

  void reduceSourceTile(CompressedTile::Ptr tile, ConstTile::Ptr const& tileData);

public:
  ////////////////////////////////////////////////////////////////////////
  /// TileInitialisationObserver
  virtual void tileFinished(CompressedTile::Ptr tile);
};

