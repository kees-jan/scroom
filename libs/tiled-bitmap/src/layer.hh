/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>

#include <scroom/tiledbitmapinterface.hh>
#include <scroom/presentationinterface.hh>

#include "tileinternal.hh"
#include "local.hh"

class Layer : public Viewable, public virtual Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<Layer> Ptr;
  
private:
  int depth;
  int width;
  int height;
  Scroom::Utils::StuffList registrations;
  int horTileCount;
  int verTileCount;
  TileInternalGrid tiles;
  TileInternal::Ptr outOfBounds;
  TileInternalLine lineOutOfBounds;

private:
  Layer(TileInitialisationObserver::Ptr observer, int depth, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider);
  
public:
  static Ptr create(TileInitialisationObserver::Ptr observer, int depth, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider);
  int getHorTileCount();
  int getVerTileCount();

  TileInternal::Ptr getTile(int i, int j);
  TileInternalLine& getTileLine(int j);
  void fetchData(SourcePresentation::Ptr sp, MultithreadingData::ConstPtr const& multithreadingData);

public:
  int getWidth()
  { return width; }

  int getHeight()
  { return height; }

  int getDepth()
  { return depth; }

public:
  // Viewable ////////////////////////////////////////////////////////////
  void open(ViewInterface::WeakPtr vi);
  void close(ViewInterface::WeakPtr vi);
};



