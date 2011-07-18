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

#include "measure-framerate-stubs.hh"

#include <boost/foreach.hpp>

ViewInterfaceStub::ViewInterfaceStub()
  :pi(new ProgressInterfaceStub())
{}

ViewInterfaceStub::~ViewInterfaceStub()
{
  delete pi;
}

ProgressInterface* ViewInterfaceStub::getProgressInterface()
{
  return pi;
}

void Source1Bpp::fillTiles(int, int lineCount, int tileWidth, int, std::vector<Tile::Ptr>& tiles)
{
  BOOST_FOREACH(Tile::Ptr tile, tiles)
  {
    byte* data = tile->data;
    for(int y=0; y<lineCount; y+=2)
    {
      for(int x=0; x<tileWidth/8; x++)
      {
        *data = 0xAA;
        data++;
      }
      for(int x=0; x<tileWidth/8; x++)
      {
        *data = 0x55;
        data++;
      }
    }
  }
}

void Source4Bpp::fillTiles(int, int lineCount, int tileWidth, int, std::vector<Tile::Ptr>& tiles)
{
  BOOST_FOREACH(Tile::Ptr tile, tiles)
  {
    byte* data = tile->data;
    for(int y=0; y<lineCount; y++)
    {
      for(int x=0; x<tileWidth/2; x++)
      {
        byte v = 2*x+y;
        
        *data = ((v&0xF) << 4) | ((v+1) & 0xF);
        data++;
      }
    }
  }
}

void Source8Bpp::fillTiles(int, int lineCount, int tileWidth, int, std::vector<Tile::Ptr>& tiles)
{
  BOOST_FOREACH(Tile::Ptr tile, tiles)
  {
    byte* data = tile->data;
    for(int y=0; y<lineCount; y++)
    {
      for(int x=0; x<tileWidth; x++)
      {
        *data = (x+y)&0xFF;
        data++;
      }
    }
  }
}

