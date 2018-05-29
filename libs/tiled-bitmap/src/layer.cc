/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "scroom/tiledbitmaplayer.hh"

#include <stdio.h>

#include <scroom/threadpool.hh>

#include "local.hh"

class DataFetcher
{
private:
  Layer::Ptr layer;
  int width;
  int height;
  int horTileCount;
  int verTileCount;
  int currentRow;
  SourcePresentation::Ptr sp;
  ThreadPool::Ptr threadPool;
  ThreadPool::WeakQueue::Ptr queue;

public:
  DataFetcher(Layer::Ptr const& layer,
              int width, int height,
              int horTileCount, int verTileCount,
              SourcePresentation::Ptr sp,
              ThreadPool::WeakQueue::Ptr queue);

  void operator()();
};

////////////////////////////////////////////////////////////////////////
/// Layer

Layer::Layer(TileInitialisationObserver::Ptr observer, int depth, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider)
  : depth(depth), width(layerWidth), height(layerHeight)
{
  horTileCount = (width+TILESIZE-1)/TILESIZE;
  verTileCount = (height+TILESIZE-1)/TILESIZE;

  for(int j=0; j<verTileCount; j++)
  {
    tiles.push_back(CompressedTileLine());
    CompressedTileLine& tl = tiles[j];
    for(int i=0; i<horTileCount; i++)
    {
      CompressedTile::Ptr tile = CompressedTile::create(depth, i, j, bpp, provider);
      registrations.push_back(tile->registerObserver(observer));
      tl.push_back(tile);
    }
  }

  outOfBounds = CompressedTile::create(depth, -1, -1, bpp, provider, TSI_OUT_OF_BOUNDS);
  for(int i=0; i<horTileCount; i++)
  {
    lineOutOfBounds.push_back(outOfBounds);
  }

  printf("Layer %d (%d bpp), %d*%d, TileCount %d*%d\n",
         depth, bpp, width, height, horTileCount, verTileCount);
}

Layer::Ptr Layer::create(TileInitialisationObserver::Ptr observer, int depth, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider)
{
  return Ptr(new Layer(observer, depth, layerWidth, layerHeight, bpp, provider));
}

int Layer::getHorTileCount()
{
  return horTileCount;
}

int Layer::getVerTileCount()
{
  return verTileCount;
}

CompressedTile::Ptr Layer::getTile(int i, int j)
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

CompressedTileLine& Layer::getTileLine(int j)
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

void Layer::fetchData(SourcePresentation::Ptr sp, ThreadPool::WeakQueue::Ptr queue)
{
  DataFetcher df(shared_from_this<Layer>(),
                 width, height,
                 horTileCount, verTileCount,
                 sp, queue);
  CpuBound()->schedule(df, DATAFETCH_PRIO, queue);
}

// Layer::Viewable /////////////////////////////////////////////////////

void Layer::open(ViewInterface::WeakPtr vi)
{
  for(CompressedTileLine& line: tiles)
  {
    for(CompressedTile::Ptr tile: line)
    {
      tile->open(vi);
    }
  }
  for(CompressedTile::Ptr tile: lineOutOfBounds)
  {
    tile->open(vi);
  }
  outOfBounds->open(vi);
}

void Layer::close(ViewInterface::WeakPtr vi)
{
  for(CompressedTileLine& line: tiles)
  {
    for(CompressedTile::Ptr tile: line)
    {
      tile->close(vi);
    }
  }
  for(CompressedTile::Ptr tile: lineOutOfBounds)
  {
    tile->close(vi);
  }
  outOfBounds->close(vi);
}

////////////////////////////////////////////////////////////////////////
/// DataFetcher

DataFetcher::DataFetcher(Layer::Ptr const& layer,
                         int width, int height,
                         int horTileCount, int verTileCount,
                         SourcePresentation::Ptr sp,
                         ThreadPool::WeakQueue::Ptr queue)
  : layer(layer), width(width), height(height),
    horTileCount(horTileCount), verTileCount(verTileCount),
    currentRow(0), sp(sp), threadPool(CpuBound()), queue(queue)
{
}

void DataFetcher::operator()()
{
  // printf("Attempting to fetch bitmap data for tileRow %d...\n", currentRow);
  QueueJumper::Ptr qj = QueueJumper::create();

  threadPool->schedule(qj, REDUCE_PRIO, queue);

  CompressedTileLine& tileLine = layer->getTileLine(currentRow);
  std::vector<Tile::Ptr> tiles;
  for(int x = 0; x < horTileCount; x++)
  {
    CompressedTile::Ptr ti = tileLine[x];
    Scroom::Utils::Stuff s = ti->initialize();
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
      threadPool->schedule(successor, DATAFETCH_PRIO, queue);
  }
  else
    sp->done();
}
