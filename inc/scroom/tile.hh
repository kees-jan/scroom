/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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

class Tile
{
public:
  typedef boost::shared_ptr<Tile> Ptr;
  typedef boost::weak_ptr<Tile> WeakPtr;
  
  int width;
  int height;
  int bpp;
  byte* data;

public:
  Tile(int width, int height, int bpp, byte* data)
    : width(width), height(height), bpp(bpp), data(data)
  {}
};

#endif
