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

#include "layer.hh"

#include <stdio.h>

#include <scroom/threadpool.hh>

#include "local.hh"

class DataFetcher
{
private:
  Layer* layer;
  int width;
  int height;
  int horTileCount;
  int verTileCount;
  int currentRow;
  SourcePresentation* sp;
  
public:
  DataFetcher(Layer* layer,
              int width, int height,
              int horTileCount, int verTileCount,
              SourcePresentation* sp,
              int currentRow = 0);

  void operator()();
};

////////////////////////////////////////////////////////////////////////
/// Layer 
Layer::Layer(TileInternalObserver* observer, int depth, int layerWidth, int layerHeight, int bpp)
  : depth(depth), width(layerWidth), height(layerHeight), bpp(bpp)
{
  horTileCount = (width+TILESIZE-1)/TILESIZE;
  verTileCount = (height+TILESIZE-1)/TILESIZE;

  for(int j=0; j<verTileCount; j++)
  {
    tiles.push_back(TileInternalLine());
    TileInternalLine& tl = tiles[j];
    for(int i=0; i<horTileCount; i++)
    {
      TileInternal* tile = new TileInternal(depth, i, j, bpp);
      tile->registerObserver(observer);
      tl.push_back(tile);
    }
  }

  outOfBounds = new TileInternal(depth, -1, -1, bpp, TILE_OUT_OF_BOUNDS);
  for(int i=0; i<horTileCount; i++)
  {
    lineOutOfBounds.push_back(outOfBounds);
  }
  
  printf("Layer %d (%d bpp), %d*%d, TileCount %d*%d\n",
         depth, bpp, width, height, horTileCount, verTileCount);
}

int Layer::getHorTileCount()
{
  return horTileCount;
}

int Layer::getVerTileCount()
{
  return verTileCount;
}

TileInternal* Layer::getTile(int i, int j)
{
  if(0<=i && i<horTileCount &&
     0<=j && j<verTileCount)
  {
    return tiles[j][i];
  }
  else
  {
    return outOfBounds;
  }
}

TileInternalLine& Layer::getTileLine(int j)
{
  if(0<=j && j<verTileCount)
  {
    return tiles[j];
  }
  else
  {
    return lineOutOfBounds;
  }
}

void Layer::fetchData(SourcePresentation* sp)
{
  DataFetcher df(this,
                 width, height,
                 horTileCount, verTileCount,
                 sp);
  schedule(df, DATAFETCH_PRIO);
}

////////////////////////////////////////////////////////////////////////
/// DataFetcher

DataFetcher::DataFetcher(Layer* layer,
                         int width, int height,
                         int horTileCount, int verTileCount,
                         SourcePresentation* sp, int currentRow)
  : layer(layer), width(width), height(height),
    horTileCount(horTileCount), verTileCount(verTileCount),
    currentRow(currentRow), sp(sp)
{
}

void DataFetcher::operator()()
{
  // printf("Attempting to fetch bitmap data for tileRow %d...\n", currentRow);
  QueueJumper::Ptr qj = QueueJumper::create();
  schedule(qj, REDUCE_PRIO);
 
  TileInternalLine& tileLine = layer->getTileLine(currentRow);
  std::vector<Tile::Ptr> tiles;
  for(int x = 0; x < horTileCount; x++)
  {
    TileInternal* ti = tileLine[x];
    ti->initialize();
    tiles.push_back(ti->getTile());
  }
  int lineCount = std::min(TILESIZE, height-currentRow*TILESIZE);

  sp->fillTiles(currentRow * TILESIZE, lineCount, TILESIZE, 0, tiles);

  for(int x = 0; x < horTileCount; x++)
  {
    tileLine[x]->reportFinished();
  }
  
  currentRow++;
  if(currentRow<verTileCount)
  {
    DataFetcher successor(*this);
    if(!qj->setWork(successor))
      schedule(successor, DATAFETCH_PRIO);
  }
}
