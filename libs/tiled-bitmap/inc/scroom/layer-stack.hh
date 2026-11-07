/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <memory>
#include <span>
#include <vector>

#include <boost/thread/mutex.hpp>

#include <scroom/layer-interface.hh>
#include <scroom/layercoordinator.hh>
#include <scroom/logger.hh>
#include <scroom/progressinterface.hh>
#include <scroom/progressinterfacehelpers.hh>
#include <scroom/stuff.hh>
#include <scroom/tiled-bitmap-data.hh>

namespace Scroom::TiledBitmap
{

  class LayerStack
    : public TileInitialisationObserver
    , public virtual Utils::Base
  {
  public:
    using Ptr = std::shared_ptr<LayerStack>;

    struct Level
    {
      LayerInterface::Ptr layer;
      int bpp;
    };

  private:
    std::vector<Level> m_levels;
    std::list<LayerCoordinator::Ptr> m_coordinators;
    Utils::StuffList m_registrations;
    Utils::ProgressInterfaceBroadcaster::Ptr m_progress{Utils::ProgressInterfaceBroadcaster::create()};
    Scroom::Logger m_logger;
    int m_tileCount{0};
    int m_finishedTileCount{0};
    boost::mutex m_progressMutex;

  public:
    static Ptr create(Utils::Point<int> size, const LayerSpec& descriptions, const LayerFactory& createLayer);

    static Ptr create(int width, int height, const LayerSpec& descriptions, const LayerFactory& createLayer)
    {
      return create(Utils::Point(width, height), descriptions, createLayer);
    }

    /**
     * Uses the supplied bottom layer. The factory creates only reduced levels.
     */
    static Ptr create(const LayerInterface::Ptr& bottom, const LayerSpec& descriptions, const LayerFactory& createLayer);

    [[nodiscard]] std::span<const Level> getLevels() const;

    void setSource(SourcePresentation::Ptr source);

    /** Retains the callback's result until the stack is destroyed. */
    void startLoading(const ReloadFunction& startLoadingCallback);

    [[nodiscard]] Utils::Stuff subscribeProgress(const ProgressInterface::Ptr& progress);

    void tileCreated(const TileStorage::Ptr& tile) override;
    void tileFinished(const TileStorage::Ptr& tile) override;

  private:
    LayerStack() = default;
    void initialize(const LayerInterface::Ptr& bottom, const LayerSpec& descriptions, const LayerFactory& createLayer);
    void connect(
      const LayerInterface::Ptr& layer,
      const LayerInterface::Ptr& previousLayer,
      const LayerReduction& reducePreviousLayer
    );
  };
} // namespace Scroom::TiledBitmap
