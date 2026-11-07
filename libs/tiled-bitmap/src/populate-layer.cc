/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <utility>
#include <vector>

#include <scroom/populate-layer.hh>
#include <scroom/tiled-bitmap-geometry.hh>

#include "local.hh"

namespace
{
  class DataFetcher
  {
  private:
    ThreadPool::Ptr m_cpuBound;
    LayerInterface::Ptr m_layer;
    SourcePresentation::Ptr m_source;
    ThreadPool::WeakQueue::Ptr m_queue;
    std::function<void()> m_onFinished;
    int m_row;

  public:
    DataFetcher(
      LayerInterface::Ptr layer,
      SourcePresentation::Ptr source,
      ThreadPool::WeakQueue::Ptr queue,
      std::function<void()> onFinished
    )
      : m_cpuBound(CpuBound())
      , m_layer(std::move(layer))
      , m_source(std::move(source))
      , m_queue(std::move(queue))
      , m_onFinished(std::move(onFinished))
      , m_row(m_layer->getTileRange().getTop())
    {
    }

    void operator()()
    {
      const auto queueJumper = QueueJumper::create();
      m_cpuBound->post([queueJumper] { std::invoke(*queueJumper); }, REDUCE_PRIO, m_queue);
      const auto columns = m_layer->getTileRange().getHorizontally();
      std::vector<Tile::Ptr> pixels;
      for(const int column: columns)
      {
        const auto tile = m_layer->getTile({column, m_row});
        const auto keepPixels = tile->initialize();
        pixels.push_back(tile->getTileSync());
      }
      const auto row = Scroom::TiledBitmap::Geometry::rowLayout(m_row, m_layer->getRect().getHeight(), TILESIZE);
      m_source->fillTiles(row.startLine, row.lineCount, TILESIZE, columns.getStart(), pixels);
      for(const int column: columns)
      {
        m_layer->getTile({column, m_row})->reportFinished();
      }
      ++m_row;
      if(m_row < m_layer->getTileRange().getBottom())
      {
        DataFetcher successor(*this);
        if(!queueJumper->setWork(successor))
        {
          m_cpuBound->post(successor, DATAFETCH_PRIO, m_queue);
        }
      }
      else
      {
        m_source->done();
        m_onFinished();
      }
    }
  };
} // namespace

void populateLayer(
  LayerInterface::Ptr layer,
  SourcePresentation::Ptr source,
  const ThreadPool::WeakQueue::Ptr& queue,
  std::function<void()> onFinished
)
{
  CpuBound()->post(DataFetcher(std::move(layer), std::move(source), queue, std::move(onFinished)), DATAFETCH_PRIO, queue);
}
