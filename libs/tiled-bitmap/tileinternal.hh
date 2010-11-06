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

#ifndef _TILEINTERNAL_HH
#define _TILEINTERNAL_HH

#include <vector>

#include <boost/thread.hpp>

#include <tiledbitmapinterface.hh>
#include <tile.hh>

#include <observable.hh>


#define TILESIZE 1024

class TileInternal;

class TileInternalObserver
{
public:
  virtual ~TileInternalObserver() {}

  virtual void tileCreated(TileInternal* tile);
  virtual void tileFinished(TileInternal* tile);
};

class TileInternal : public Observable<TileInternalObserver>
{
public:
  int depth;
  int x;
  int y;
  int bpp;
  TileState state;
  Tile::WeakPtr tile;
  byte* data;
  boost::mutex mut;
  
public:
  TileInternal(int depth, int x, int y, int bpp, TileState state=TILE_UNINITIALIZED);

  void initialize();
  
  void registerObserver(TileInternalObserver* observer);

  Tile::Ptr getTile();

  void reportFinished();
};

typedef std::vector<TileInternal*> TileInternalLine;
typedef std::vector<TileInternalLine> TileInternalGrid;

#endif
