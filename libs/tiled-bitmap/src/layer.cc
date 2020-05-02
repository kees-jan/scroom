/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
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
  int height;
  int horTileCount;
  int verTileCount;
  int currentRow;
  SourcePresentation::Ptr sp;
  ThreadPool::Ptr threadPool;
  ThreadPool::WeakQueue::Ptr queue;

public:
  DataFetcher(Layer::Ptr const& layer,
              int height,
              int horTileCount, int verTileCount,
              SourcePresentation::Ptr sp,
              ThreadPool::WeakQueue::Ptr queue);

  void operator()();
};

////////////////////////////////////////////////////////////////////////
/// Layer

Layer::Layer(TileInitialisationObserver::Ptr observer, int depth_, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider)
  : depth(depth_), width(layerWidth), height(layerHeight)
{
  horTileCount = (width+TILESIZE-1)/TILESIZE;
  verTileCount = (height+TILESIZE-1)/TILESIZE;

  for(size_t j=0; j<static_cast<size_t>(verTileCount); j++)
  {
    tiles.push_back(CompressedTileLine());
    CompressedTileLine& tl = tiles[j];
    for(int i=0; i<horTileCount; i++)
    {
      CompressedTile::Ptr tile = CompressedTile::create(depth_, i, static_cast<int>(j), bpp, provider);
      registrations.push_back(tile->registerObserver(observer));
      tl.push_back(tile);
    }
  }

  outOfBounds = CompressedTile::create(depth_, -1, -1, bpp, provider, TSI_OUT_OF_BOUNDS);
  for(int i=0; i<horTileCount; i++)
  {
    lineOutOfBounds.push_back(outOfBounds);
  }

  printf("Layer %d (%d bpp), %d*%d, TileCount %d*%d\n",
         depth_, bpp, width, height, horTileCount, verTileCount);
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
    return tiles[static_cast<size_t>(j)][static_cast<size_t>(i)];
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
    return tiles[static_cast<size_t>(j)];
  }
  else
  {
    return lineOutOfBounds;
  }
}

void Layer::fetchData(SourcePresentation::Ptr sp, ThreadPool::WeakQueue::Ptr queue)
{
  DataFetcher df(shared_from_this<Layer>(),
                 height,
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

DataFetcher::DataFetcher(Layer::Ptr const& layer_,
                         int height_,
                         int horTileCount_, int verTileCount_,
                         SourcePresentation::Ptr sp_,
                         ThreadPool::WeakQueue::Ptr queue_)
  : layer(layer_), height(height_),
    horTileCount(horTileCount_), verTileCount(verTileCount_),
    currentRow(0), sp(sp_), threadPool(CpuBound()), queue(queue_)
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
    CompressedTile::Ptr ti = tileLine[static_cast<size_t>(x)];
    Scroom::Utils::Stuff s = ti->initialize();
    tiles.push_back(ti->getTileSync());
  }
  int lineCount = std::min(TILESIZE, height-currentRow*TILESIZE);

  sp->fillTiles(currentRow * TILESIZE, lineCount, TILESIZE, 0, tiles);

  for(int x = 0; x < horTileCount; x++)
  {
    tileLine[static_cast<size_t>(x)]->reportFinished();
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
