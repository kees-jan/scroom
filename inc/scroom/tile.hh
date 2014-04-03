/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

#ifndef _TILE_HH
#define _TILE_HH

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <scroom/global.hh>
#include <scroom/memoryblobs.hh>

class Tile
{
public:
  typedef boost::shared_ptr<Tile> Ptr;
  typedef boost::weak_ptr<Tile> WeakPtr;
  
  int width;
  int height;
  int bpp;
  Scroom::MemoryBlobs::RawPageData::Ptr data;

public:
  Tile(int width, int height, int bpp, Scroom::MemoryBlobs::RawPageData::Ptr data)
    : width(width), height(height), bpp(bpp), data(data)
  {}

  static Tile::Ptr create(int width, int height, int bpp, Scroom::MemoryBlobs::RawPageData::Ptr data)
  { return Ptr(new Tile(width, height, bpp, data)); }
};

class ConstTile
{
public:
  typedef boost::shared_ptr<ConstTile> Ptr;
  typedef boost::weak_ptr<ConstTile> WeakPtr;
  
  int width;
  int height;
  int bpp;
  Scroom::MemoryBlobs::RawPageData::ConstPtr data;

public:
  ConstTile(int width, int height, int bpp, Scroom::MemoryBlobs::RawPageData::ConstPtr data)
    : width(width), height(height), bpp(bpp), data(data)
  {}

  static ConstTile::Ptr create(int width, int height, int bpp, Scroom::MemoryBlobs::RawPageData::ConstPtr data)
  { return Ptr(new ConstTile(width, height, bpp, data)); }
};

#endif
