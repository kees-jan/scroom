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

#ifndef _LAYER_HH
#define _LAYER_HH

#include <vector>

#include <scroom/tiledbitmapinterface.hh>
#include <scroom/presentationinterface.hh>

#include "tileinternal.hh"

class Layer : public Viewable
{
private:
  int depth;
  int width;
  int height;
  int bpp;
  Scroom::Utils::StuffList registrations;
  int horTileCount;
  int verTileCount;
  TileInternalGrid tiles;
  TileInternal::Ptr outOfBounds;
  TileInternalLine lineOutOfBounds;
  
public:
  Layer(TileInitialisationObserver::Ptr observer, int depth, int layerWidth, int layerHeight, int bpp);
  int getHorTileCount();
  int getVerTileCount();

  TileInternal::Ptr getTile(int i, int j);
  TileInternalLine& getTileLine(int j);
  void fetchData(SourcePresentation* sp, ThreadPool::WeakQueue::Ptr queue);

public:
  int getWidth()
  { return width; }

  int getHeight()
  { return height; }

  int getDepth()
  { return depth; }

public:
  // Viewable ////////////////////////////////////////////////////////////
  void open(ViewInterface* vi);
  void close(ViewInterface* vi);
};


#endif
