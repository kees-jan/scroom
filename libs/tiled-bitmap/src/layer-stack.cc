/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <algorithm>
#include <list>
#include <utility>
#include <vector>

#include <boost/thread/mutex.hpp>

#include <scroom/assertions.hh>
#include <scroom/layer-stack.hh>
#include <scroom/layercoordinator.hh>
#include <scroom/logger.hh>
#include <scroom/opentiledbitmapinterface.hh>
#include <scroom/progressinterfacehelpers.hh>
#include <scroom/semaphore.hh>
#include <scroom/tiled-bitmap-geometry.hh>

namespace Scroom::TiledBitmap
{
  Utils::Stuff
    scheduleLoadingBitmap(const SourcePresentation::Ptr& sp, const Layer::Ptr& layer, const ProgressInterface::Ptr& progress)
  {
    auto wait_until_done = std::make_shared<Semaphore>();
    auto queue = ThreadPool::Queue::createAsync();
    auto weakQueue = queue->getWeak();
    auto abort = [wait_until_done, queue]() mutable
    {
      queue.reset();
      wait_until_done->V();
    };

    auto on_finished = [wait_until_done] { wait_until_done->V(); };
    progress->setWaiting();

    Sequentially()->post(
      [progress, layer, sp, weakQueue, on_finished, wait_until_done]
      {
        progress->setWorking(0);
        layer->fetchData(sp, weakQueue, on_finished);
        wait_until_done->P();
      }
    );

    return Utils::on_destruction(abort);
  }

  void LayerStack::initialize(const Layer::Ptr& bottom, const LayerSpec& descriptions)
  {
    const auto sizes = Geometry::pyramidSizes({bottom->getWidth(), bottom->getHeight()}, TILESIZE / 4);
    auto reducePreviousLayer = descriptions.front().reduction;
    const auto provider = bottom->getPageProvider();
    m_registrations.emplace_back(bottom->registerObserver(shared_from_this<TileInitialisationObserver>()));
    m_levels.push_back({bottom, descriptions.front().bpp});
    auto previousLayer = bottom;

    for(std::size_t i = 1; i < sizes.size(); ++i)
    {
      const auto& description = descriptions[std::min(i, descriptions.size() - 1)];
      const auto layer = Layer::create(i, sizes[i].x, sizes[i].y, description.bpp, provider);
      m_registrations.emplace_back(layer->registerObserver(shared_from_this<TileInitialisationObserver>()));
      m_levels.push_back({layer, description.bpp});
      connect(layer, previousLayer, reducePreviousLayer);
      previousLayer = layer;
      reducePreviousLayer = description.reduction;
    }
  }

  void LayerStack::tileCreated(const CompressedTile::Ptr& /*tile*/) { ++m_tileCount; }

  void LayerStack::tileFinished(const CompressedTile::Ptr& /*tile*/)
  {
    boost::mutex::scoped_lock const lock(m_progressMutex);
    ++m_finishedTileCount;
    require(m_finishedTileCount <= m_tileCount);
    m_progress->setWorking(1.0 * m_finishedTileCount / m_tileCount);
    if(m_finishedTileCount == m_tileCount)
    {
      m_progress->setFinished();
      m_logger->info("Finished loading file");
    }
  }


  void LayerStack::connect(Layer::Ptr const& layer, Layer::Ptr const& prevLayer, const LayerReduction& reducePreviousLayer)
  {
    const int horTileCount = prevLayer->getHorTileCount();
    const int verTileCount = prevLayer->getVerTileCount();

    std::vector<LayerCoordinator::Ptr> coordinators_;

    for(int j = 0; j < verTileCount; j++)
    {
      const auto rowMapping = Geometry::reductionMapping({0, j});
      const int voffset = rowMapping.sourceOffset.y;
      if(!voffset)
      {
        // New line of target tiles
        coordinators_.clear();
        CompressedTileLine const& tileLine = layer->getTileLine(rowMapping.targetIndex.y);
        for(auto& z: tileLine)
        {
          LayerCoordinator::Ptr const lc = LayerCoordinator::create(z, reducePreviousLayer);
          coordinators_.push_back(lc);
          m_coordinators.push_back(lc);
        }
      }

      for(int i = 0; i < horTileCount; i++)
      {
        const auto mapping = Geometry::reductionMapping({i, j});
        LayerCoordinator::Ptr const lc = coordinators_[mapping.targetIndex.x];
        lc->addSourceTile(mapping.sourceOffset, prevLayer->getTile({i, j}));
      }
    }
  }


  LayerStack::Ptr LayerStack::create(Utils::Point<int> size, const LayerSpec& descriptions)
  {
    require(!descriptions.empty());
    return create(Layer::create(size.x, size.y, descriptions.front().bpp), descriptions);
  }

  LayerStack::Ptr LayerStack::create(const Layer::Ptr& bottom, const LayerSpec& descriptions)
  {
    require(bottom);
    require(!descriptions.empty());
    Ptr stack(new LayerStack());
    stack->initialize(bottom, descriptions);
    return stack;
  }

  std::span<const LayerStack::Level> LayerStack::getLevels() const { return m_levels; }

  void LayerStack::setSource(SourcePresentation::Ptr source)
  {
    startLoading([source = std::move(source), bottom = m_levels.front().layer](const ProgressInterface::Ptr& progress)
                 { return scheduleLoadingBitmap(source, bottom, progress); });
  }

  void LayerStack::startLoading(const ReloadFunction& startLoading) { m_registrations.push_back(startLoading(m_progress)); }

  Utils::Stuff LayerStack::subscribeProgress(const ProgressInterface::Ptr& progress) { return m_progress->subscribe(progress); }
} // namespace Scroom::TiledBitmap
