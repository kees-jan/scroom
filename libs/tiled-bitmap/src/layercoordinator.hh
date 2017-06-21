/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <map>
#include <utility>

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "tileinternal.hh"
#include "local.hh"

class LayerCoordinator: public TileInitialisationObserver,
                        public virtual Scroom::Utils::Base
{
private:
  TileInternal::Ptr targetTile;
  Tile::Ptr targetTileData;
  std::map<TileInternal::Ptr,std::pair<int,int> > sourceTiles;
  Scroom::Utils::StuffList registrations;
  LayerOperations::Ptr lo;
  boost::mutex mut;
  int unfinishedSourceTiles;
  MultithreadingData::ConstPtr multithreadingData;

public:
  typedef boost::shared_ptr<LayerCoordinator> Ptr;
  
  static Ptr create(TileInternal::Ptr targetTile, LayerOperations::Ptr lo,
                    MultithreadingData::ConstPtr const& multithreadingData);
  
  virtual ~LayerCoordinator();
  
  void addSourceTile(int x, int y, TileInternal::Ptr tile);

private:
  LayerCoordinator(TileInternal::Ptr targetTile, LayerOperations::Ptr lo,
                   MultithreadingData::ConstPtr const& multithreadingData);

  void reduceSourceTile(TileInternal::Ptr tile, ConstTile::Ptr const& tileData);
  
public:
  ////////////////////////////////////////////////////////////////////////
  /// TileInitialisationObserver
  virtual void tileFinished(TileInternal::Ptr tile);
};


