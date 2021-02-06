/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <vector>

#include <stdio.h>

#include <scroom/impl/threadpoolimpl.hh>
#include <scroom/memoryblobs.hh>
#include <scroom/stuff.hh>
#include <scroom/threadpool.hh>
#include <scroom/tile.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/tiledbitmaplayer.hh>
#include <scroom/viewinterface.hh>

#include "local.hh"

class DataFetcher
{
private:
  Layer::Ptr                 layer;
  int                        height;
  int                        horTileCount;
  int                        verTileCount;
  int                        currentRow;
  SourcePresentation::Ptr    sp;
  ThreadPool::Ptr            threadPool;
  ThreadPool::WeakQueue::Ptr queue;

public:
  DataFetcher(Layer::Ptr const&          layer,
              int                        height,
              int                        horTileCount,
              int                        verTileCount,
              SourcePresentation::Ptr    sp,
              ThreadPool::WeakQueue::Ptr queue);

  void operator()();
};

////////////////////////////////////////////////////////////////////////
/// Layer

Layer::Layer(int depth_, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider)
  : depth(depth_)
  , width(layerWidth)
  , height(layerHeight)
{
  horTileCount = (width + TILESIZE - 1) / TILESIZE;
  verTileCount = (height + TILESIZE - 1) / TILESIZE;

  for(int j = 0; j < verTileCount; j++)
  {
    tiles.push_back(CompressedTileLine());
    CompressedTileLine& tl = tiles[j];
    for(int i = 0; i < horTileCount; i++)
    {
      CompressedTile::Ptr tile = CompressedTile::create(depth_, i, j, bpp, provider);
      tl.push_back(tile);
    }
  }

  outOfBounds = CompressedTile::create(depth_, -1, -1, bpp, provider, TSI_OUT_OF_BOUNDS);
  for(int i = 0; i < horTileCount; i++)
  {
    lineOutOfBounds.push_back(outOfBounds);
  }

  printf("Layer %d (%d bpp), %d*%d, TileCount %d*%d\n", depth_, bpp, width, height, horTileCount, verTileCount);
}

Layer::Ptr Layer::create(int depth, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider)
{
  return Ptr(new Layer(depth, layerWidth, layerHeight, bpp, provider));
}

Scroom::Bookkeeping::Token Layer::registerObserver(const TileInitialisationObserver::Ptr& observer)
{
  Scroom::Bookkeeping::Token t;

  for(auto& line: tiles)
  {
    for(auto& tile: line)
    {
      t.add(tile->registerObserver(observer));
    }
  }
  return t;
}

int Layer::getHorTileCount() const { return horTileCount; }

int Layer::getVerTileCount() const { return verTileCount; }

CompressedTile::Ptr Layer::getTile(int i, int j)
{
  if(0 <= i && i < horTileCount && 0 <= j && j < verTileCount)
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
  if(0 <= j && j < verTileCount)
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
  DataFetcher df(shared_from_this<Layer>(), height, horTileCount, verTileCount, sp, queue);
  CpuBound()->schedule(df, DATAFETCH_PRIO, queue);
}

// Layer::Viewable /////////////////////////////////////////////////////

void Layer::open(ViewInterface::WeakPtr vi)
{
  for(CompressedTileLine& line: tiles)
  {
    for(const CompressedTile::Ptr& tile: line)
    {
      tile->open(vi);
    }
  }
  for(const CompressedTile::Ptr& tile: lineOutOfBounds)
  {
    tile->open(vi);
  }
  outOfBounds->open(vi);
}

void Layer::close(ViewInterface::WeakPtr vi)
{
  for(CompressedTileLine& line: tiles)
  {
    for(const CompressedTile::Ptr& tile: line)
    {
      tile->close(vi);
    }
  }
  for(const CompressedTile::Ptr& tile: lineOutOfBounds)
  {
    tile->close(vi);
  }
  outOfBounds->close(vi);
}

////////////////////////////////////////////////////////////////////////
/// DataFetcher

DataFetcher::DataFetcher(Layer::Ptr const&          layer_,
                         int                        height_,
                         int                        horTileCount_,
                         int                        verTileCount_,
                         SourcePresentation::Ptr    sp_,
                         ThreadPool::WeakQueue::Ptr queue_)
  : layer(layer_)
  , height(height_)
  , horTileCount(horTileCount_)
  , verTileCount(verTileCount_)
  , currentRow(0)
  , sp(sp_)
  , threadPool(CpuBound())
  , queue(queue_)
{}

void DataFetcher::operator()()
{
  // printf("Attempting to fetch bitmap data for tileRow %d...\n", currentRow);
  QueueJumper::Ptr qj = QueueJumper::create();

  threadPool->schedule(qj, REDUCE_PRIO, queue);

  CompressedTileLine&    tileLine = layer->getTileLine(currentRow);
  std::vector<Tile::Ptr> tiles;
  for(int x = 0; x < horTileCount; x++)
  {
    CompressedTile::Ptr  ti = tileLine[x];
    Scroom::Utils::Stuff s  = ti->initialize();
    tiles.push_back(ti->getTileSync());
  }
  int lineCount = std::min(TILESIZE, height - currentRow * TILESIZE);

  sp->fillTiles(currentRow * TILESIZE, lineCount, TILESIZE, 0, tiles);

  for(int x = 0; x < horTileCount; x++)
  {
    tileLine[x]->reportFinished();
  }

  currentRow++;
  if(currentRow < verTileCount)
  {
    DataFetcher successor(*this);
    if(!qj->setWork(successor))
    {
      threadPool->schedule(successor, DATAFETCH_PRIO, queue);
    }
  }
  else
  {
    sp->done();
  }
}
