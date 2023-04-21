/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstdio>
#include <utility>
#include <vector>

#include <spdlog/spdlog.h>

#include <scroom/impl/threadpoolimpl.hh>
#include <scroom/memoryblobs.hh>
#include <scroom/stuff.hh>
#include <scroom/threadpool.hh>
#include <scroom/tile.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/tiledbitmaplayer.hh>
#include <scroom/viewinterface.hh>

#include "local.hh"

static Scroom::MemoryBlobs::PageProvider::Ptr createProvider(double width, double height, int bpp)
{
  const double tileCount = (width * height) / (TILESIZE * TILESIZE);
  const double tileSize  = (bpp / 8.0) * TILESIZE * TILESIZE;

  const double guessedTileSizeAfterCompression = tileSize / 100;
  const int    pagesize                        = 4096;
  const int    pagesPerBlock                   = std::max(int(ceil(guessedTileSizeAfterCompression / 10 / pagesize)), 1);

  const int blockSize  = pagesPerBlock * pagesize;
  const int blockCount = std::max(int(ceil(tileCount / 10)), 64);

  spdlog::debug("Creating a PageProvider providing {} blocks of {} bytes", blockCount, blockSize);
  return Scroom::MemoryBlobs::PageProvider::create(blockCount, blockSize);
}


class DataFetcher
{
private:
  Layer::Ptr                 layer;
  int                        height;
  int                        horTileCount;
  int                        verTileCount;
  int                        currentRow{0};
  SourcePresentation::Ptr    sp;
  ThreadPool::Ptr            threadPool;
  ThreadPool::WeakQueue::Ptr queue;
  std::function<void()>      on_finished;

public:
  DataFetcher(Layer::Ptr                 layer,
              int                        height,
              int                        horTileCount,
              int                        verTileCount,
              SourcePresentation::Ptr    sp,
              ThreadPool::WeakQueue::Ptr queue,
              std::function<void()>      on_finished);

  void operator()();
};

////////////////////////////////////////////////////////////////////////
/// Layer

Layer::Layer(int depth_, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider)
  : depth(depth_)
  , width(layerWidth)
  , height(layerHeight)
  , horTileCount((width + TILESIZE - 1) / TILESIZE)
  , verTileCount((height + TILESIZE - 1) / TILESIZE)
  , pageProvider(std::move(provider))
{
  for(int j = 0; j < verTileCount; j++)
  {
    tiles.emplace_back();
    CompressedTileLine& tl = tiles[j];
    for(int i = 0; i < horTileCount; i++)
    {
      CompressedTile::Ptr const tile = CompressedTile::create(depth_, i, j, bpp, pageProvider);
      tl.push_back(tile);
    }
  }

  outOfBounds = CompressedTile::create(depth_, -1, -1, bpp, pageProvider, TSI_OUT_OF_BOUNDS);
  for(int i = 0; i < horTileCount; i++)
  {
    lineOutOfBounds.push_back(outOfBounds);
  }

  spdlog::debug("Layer {} ({} bpp), {}*{}, TileCount {}*{}", depth_, bpp, width, height, horTileCount, verTileCount);
}

Layer::Ptr Layer::create(int depth, int layerWidth, int layerHeight, int bpp, Scroom::MemoryBlobs::PageProvider::Ptr provider)
{
  return Ptr(new Layer(depth, layerWidth, layerHeight, bpp, std::move(provider)));
}

Layer::Ptr Layer::create(int layerWidth, int layerHeight, int bpp)
{
  Scroom::MemoryBlobs::PageProvider::Ptr const provider = createProvider(layerWidth, layerHeight, bpp);

  return create(0, layerWidth, layerHeight, bpp, provider);
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

  return outOfBounds;
}

CompressedTileLine& Layer::getTileLine(int j)
{
  if(0 <= j && j < verTileCount)
  {
    return tiles[j];
  }

  return lineOutOfBounds;
}

void Layer::fetchData(SourcePresentation::Ptr sp, const ThreadPool::WeakQueue::Ptr& queue, std::function<void()> on_finished)
{
  DataFetcher const df(
    shared_from_this<Layer>(), height, horTileCount, verTileCount, std::move(sp), queue, std::move(on_finished));
  CpuBound()->schedule(df, DATAFETCH_PRIO, queue);
}

// Layer::Viewable /////////////////////////////////////////////////////

void Layer::open(ViewInterface::WeakPtr vi)
{
  for(CompressedTileLine const& line: tiles)
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
  for(CompressedTileLine const& line: tiles)
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

DataFetcher::DataFetcher(Layer::Ptr                 layer_,
                         int                        height_,
                         int                        horTileCount_,
                         int                        verTileCount_,
                         SourcePresentation::Ptr    sp_,
                         ThreadPool::WeakQueue::Ptr queue_,
                         std::function<void()>      on_finished_)
  : layer(std::move(layer_))
  , height(height_)
  , horTileCount(horTileCount_)
  , verTileCount(verTileCount_)
  , sp(std::move(sp_))
  , threadPool(CpuBound())
  , queue(std::move(queue_))
  , on_finished(std::move(on_finished_))
{
}

void DataFetcher::operator()()
{
  QueueJumper::Ptr const qj = QueueJumper::create();

  threadPool->schedule(qj, REDUCE_PRIO, queue);

  CompressedTileLine&    tileLine = layer->getTileLine(currentRow);
  std::vector<Tile::Ptr> tiles;
  for(int x = 0; x < horTileCount; x++)
  {
    CompressedTile::Ptr const  ti = tileLine[x];
    Scroom::Utils::Stuff const s  = ti->initialize();
    tiles.push_back(ti->getTileSync());
  }
  const int lineCount = std::min(TILESIZE, height - currentRow * TILESIZE);

  sp->fillTiles(currentRow * TILESIZE, lineCount, TILESIZE, 0, tiles);

  for(int x = 0; x < horTileCount; x++)
  {
    tileLine[x]->reportFinished();
  }

  currentRow++;
  if(currentRow < verTileCount)
  {
    DataFetcher const successor(*this);
    if(!qj->setWork(successor))
    {
      threadPool->schedule(successor, DATAFETCH_PRIO, queue);
    }
  }
  else
  {
    sp->done();
    on_finished();
  }
}
