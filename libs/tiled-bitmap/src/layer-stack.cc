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
#include <scroom/populate-layer.hh>
#include <scroom/progressinterfacehelpers.hh>
#include <scroom/semaphore.hh>
#include <scroom/tiled-bitmap-geometry.hh>

namespace Scroom::TiledBitmap
{
  Utils::Stuff scheduleLoadingBitmap(
    const SourcePresentation::Ptr& sp,
    const LayerInterface::Ptr& layer,
    const ProgressInterface::Ptr& progress
  )
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
        populateLayer(layer, sp, weakQueue, on_finished);
        wait_until_done->P();
      }
    );

    return Utils::on_destruction(abort);
  }

  void LayerStack::initialize(const LayerInterface::Ptr& bottom, const LayerSpec& descriptions, const LayerFactory& createLayer)
  {
    const auto imageArea = bottom->getRect();
    const auto sizes = Geometry::pyramidSizes({imageArea.getWidth(), imageArea.getHeight()}, TILESIZE / 4);
    auto reducePreviousLayer = descriptions.front().reduction;
    m_registrations.emplace_back(bottom->registerObserver(shared_from_this<TileInitialisationObserver>()));
    m_levels.push_back({bottom, descriptions.front().bpp});
    auto previousLayer = bottom;

    for(std::size_t i = 1; i < sizes.size(); ++i)
    {
      const auto& description = descriptions[std::min(i, descriptions.size() - 1)];
      const auto layer = createLayer(i, Geometry::reducedBounds(previousLayer->getRect()), description.bpp);
      verify(layer);
      m_registrations.emplace_back(layer->registerObserver(shared_from_this<TileInitialisationObserver>()));
      m_levels.push_back({layer, description.bpp});
      connect(layer, previousLayer, reducePreviousLayer);
      previousLayer = layer;
      reducePreviousLayer = description.reduction;
    }
  }

  void LayerStack::tileCreated(const TileStorage::Ptr& /*tile*/) { ++m_tileCount; }

  void LayerStack::tileFinished(const TileStorage::Ptr& /*tile*/)
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


  void LayerStack::connect(
    LayerInterface::Ptr const& layer,
    LayerInterface::Ptr const& prevLayer,
    const LayerReduction& reducePreviousLayer
  )
  {
    std::map<TileStorage::Ptr, LayerCoordinator::Ptr> targets;
    for(const auto sourceIndex: prevLayer->getTileRange())
    {
      const auto mapping = Geometry::reductionMapping(sourceIndex);
      const auto target = layer->getTile(mapping.targetIndex);
      auto& coordinator = targets[target];
      if(!coordinator)
      {
        coordinator = LayerCoordinator::create(target, reducePreviousLayer);
        m_coordinators.push_back(coordinator);
      }
      coordinator->addSourceTile(mapping.sourceOffset, prevLayer->getTile(sourceIndex));
    }
  }


  LayerStack::Ptr LayerStack::create(Utils::Point<int> size, const LayerSpec& descriptions, const LayerFactory& createLayer)
  {
    require(!descriptions.empty());
    require(createLayer);
    const auto bottom = createLayer(0, Utils::Rectangle(0, 0, size.x, size.y), descriptions.front().bpp);
    return create(bottom, descriptions, createLayer);
  }

  LayerStack::Ptr
    LayerStack::create(const LayerInterface::Ptr& bottom, const LayerSpec& descriptions, const LayerFactory& createLayer)
  {
    require(bottom);
    require(!descriptions.empty());
    Ptr stack(new LayerStack());
    require(bottom->getRect().getTopLeft() == Utils::Point(0, 0));
    require(createLayer);
    stack->initialize(bottom, descriptions, createLayer);
    return stack;
  }

  std::span<const LayerStack::Level> LayerStack::getLevels() const { return m_levels; }

  void LayerStack::setSource(SourcePresentation::Ptr source)
  {
    startLoading([source = std::move(source), bottom = m_levels.front().layer](const ProgressInterface::Ptr& progress)
                 { return scheduleLoadingBitmap(source, bottom, progress); });
  }

  void LayerStack::startLoading(const ReloadFunction& startLoadingCallback)
  {
    m_registrations.push_back(startLoadingCallback(m_progress));
  }

  Utils::Stuff LayerStack::subscribeProgress(const ProgressInterface::Ptr& progress) { return m_progress->subscribe(progress); }
} // namespace Scroom::TiledBitmap
