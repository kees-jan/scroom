/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <map>
#include <memory>
#include <utility>

#include <boost/thread.hpp>

#include <scroom/tiledbitmaplayer.hh>

class LayerCoordinator
  : public TileInitialisationObserver
  , public virtual Scroom::Utils::Base
{
private:
  CompressedTile::Ptr                                targetTile;
  Tile::Ptr                                          targetTileData;
  std::map<CompressedTile::Ptr, std::pair<int, int>> sourceTiles;
  Scroom::Utils::StuffList                           registrations;
  LayerOperations::Ptr                               lo;
  boost::mutex                                       mut;
  int                                                unfinishedSourceTiles{0};

public:
  using Ptr = std::shared_ptr<LayerCoordinator>;

  static Ptr create(CompressedTile::Ptr targetTile, LayerOperations::Ptr lo);

  ~LayerCoordinator() override;
  LayerCoordinator(const LayerCoordinator&)           = delete;
  LayerCoordinator(LayerCoordinator&&)                = delete;
  LayerCoordinator operator=(const LayerCoordinator&) = delete;
  LayerCoordinator operator=(LayerCoordinator&&)      = delete;

  void addSourceTile(int x, int y, const CompressedTile::Ptr& tile);

private:
  LayerCoordinator(CompressedTile::Ptr targetTile, LayerOperations::Ptr lo);

  void reduceSourceTile(const CompressedTile::Ptr& tile, ConstTile::Ptr const& tileData);

public:
  ////////////////////////////////////////////////////////////////////////
  /// TileInitialisationObserver
  void tileFinished(const CompressedTile::Ptr& tile) override;
};
