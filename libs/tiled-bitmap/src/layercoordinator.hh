/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "tileinternal.hh"

class LayerCoordinator: public TileInitialisationObserver,
                        public boost::enable_shared_from_this<LayerCoordinator>
{
private:
  TileInternal::Ptr targetTile;
  std::map<TileInternal::Ptr,std::pair<int,int> > sourceTiles;
  Scroom::Utils::StuffList registrations;
  LayerOperations* lo;
  boost::mutex mut;
  int unfinishedSourceTiles;

public:
  typedef boost::shared_ptr<LayerCoordinator> Ptr;
  
  static Ptr create(TileInternal::Ptr targetTile, LayerOperations* lo);
  
  virtual ~LayerCoordinator();
  
  void addSourceTile(int x, int y, TileInternal::Ptr tile);

private:
  LayerCoordinator(TileInternal::Ptr targetTile, LayerOperations* lo);

public:
  ////////////////////////////////////////////////////////////////////////
  /// TileInitialisationObserver
  virtual void tileFinished(TileInternal::Ptr tile);
};

#endif
