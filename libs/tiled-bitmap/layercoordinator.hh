/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

#ifndef LAYERCOORDINATOR_HH
#define LAYERCOORDINATOR_HH

#include <map>
#include <utility>

#include <boost/thread.hpp>

#include "tileinternal.hh"

class LayerCoordinator: private TileInternalObserver
{
private:
  TileInternal* targetTile;
  std::map<TileInternal*,std::pair<int,int> > sourceTiles;
  LayerOperations* lo;
  boost::mutex mut;
  int unfinishedSourceTiles;

public:
  LayerCoordinator(TileInternal* targetTile, LayerOperations* lo);
  virtual ~LayerCoordinator();
  
  void addSourceTile(int x, int y, TileInternal* tile);

private:
  ////////////////////////////////////////////////////////////////////////
  /// TileInternalObserver
  virtual void tileFinished(TileInternal* tile);
};

#endif
