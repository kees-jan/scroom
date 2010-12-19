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
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <scroom/tiledbitmapinterface.hh>
#include <scroom/tile.hh>

#include <scroom/observable.hh>

#include <scroom/memorymanagerinterface.hh>


#define TILESIZE 4096
// #define TILESIZE 1024

class TileInternal;

class TileInternalObserver
{
public:
  virtual ~TileInternalObserver() {}

  virtual void tileCreated(boost::shared_ptr<TileInternal> tile);
  virtual void tileFinished(boost::shared_ptr<TileInternal> tile);
};

class TileInternal : public Observable<TileInternalObserver>, public MemoryManagedInterface,
                     public boost::enable_shared_from_this<TileInternal>
{
public:
  typedef boost::shared_ptr<TileInternal> Ptr;
  
public:
  int depth;
  int x;
  int y;
  int bpp;
  TileState state;
  Tile::WeakPtr tile;
  FileBackedMemory data;
  boost::mutex mut;

private:
  TileInternal(int depth, int x, int y, int bpp, TileState state);

public:
  static Ptr create(int depth, int x, int y, int bpp, TileState state=TILE_UNINITIALIZED);

  void initialize();
  
  void registerObserver(TileInternalObserver* observer);

  Tile::Ptr getTile();

  void reportFinished();

  // MemoryManagedInterface //////////////////////////////////////////////
  virtual bool do_unload();
};

typedef std::vector<TileInternal::Ptr> TileInternalLine;
typedef std::vector<TileInternalLine> TileInternalGrid;

#endif
