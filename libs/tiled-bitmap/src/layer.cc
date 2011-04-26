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
  ThreadPool::Ptr threadPool;
  ThreadPool::Queue::WeakPtr queue;
  
public:
  DataFetcher(Layer* layer,
              int width, int height,
              int horTileCount, int verTileCount,
              SourcePresentation* sp,
              ThreadPool::Queue::Ptr queue);

  void operator()();
};

////////////////////////////////////////////////////////////////////////
/// Layer 
Layer::Layer(TileInitialisationObserver::Ptr observer, int depth, int layerWidth, int layerHeight, int bpp)
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
      TileInternal::Ptr tile = TileInternal::create(depth, i, j, bpp);
      registrations.push_back(tile->registerObserver(observer));
      tl.push_back(tile);
    }
  }

  outOfBounds = TileInternal::create(depth, -1, -1, bpp, TSI_OUT_OF_BOUNDS);
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

TileInternal::Ptr Layer::getTile(int i, int j)
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

void Layer::fetchData(SourcePresentation* sp, ThreadPool::Queue::Ptr queue)
{
  DataFetcher df(this,
                 width, height,
                 horTileCount, verTileCount,
                 sp, queue);
  CpuBound()->schedule(df, DATAFETCH_PRIO, queue);
}

////////////////////////////////////////////////////////////////////////
/// DataFetcher

DataFetcher::DataFetcher(Layer* layer,
                         int width, int height,
                         int horTileCount, int verTileCount,
                         SourcePresentation* sp,
                         ThreadPool::Queue::Ptr queue)
  : layer(layer), width(width), height(height),
    horTileCount(horTileCount), verTileCount(verTileCount),
    currentRow(0), sp(sp), threadPool(CpuBound()), queue(queue)
{
}

void DataFetcher::operator()()
{
  ThreadPool::Queue::Ptr q(queue);

  // printf("Attempting to fetch bitmap data for tileRow %d...\n", currentRow);
  QueueJumper::Ptr qj = QueueJumper::create();

  threadPool->schedule(qj, REDUCE_PRIO, q);
 
  TileInternalLine& tileLine = layer->getTileLine(currentRow);
  std::vector<Tile::Ptr> tiles;
  for(int x = 0; x < horTileCount; x++)
  {
    TileInternal::Ptr ti = tileLine[x];
    ti->initialize();
    tiles.push_back(ti->getTileSync());
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
      threadPool->schedule(successor, DATAFETCH_PRIO, q);
  }
  else
    sp->done();
}
