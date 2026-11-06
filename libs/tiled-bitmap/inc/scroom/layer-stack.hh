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

#include <scroom/layercoordinator.hh>
#include <scroom/logger.hh>
#include <scroom/progressinterface.hh>
#include <scroom/progressinterfacehelpers.hh>
#include <scroom/stuff.hh>
#include <scroom/tiled-bitmap-data.hh>
#include <scroom/tiledbitmaplayer.hh>

namespace Scroom::TiledBitmap
{

  /**
   * Owns the default layer pyramid and its population work independently of views.
   * Population currently supports a single pass.
   */
  class LayerStack
    : public TileInitialisationObserver
    , public virtual Utils::Base
  {
  public:
    using Ptr = std::shared_ptr<LayerStack>;

    struct Level
    {
      Layer::Ptr layer;
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
    static Ptr create(Utils::Point<int> size, const LayerSpec& descriptions);

    static Ptr create(int width, int height, const LayerSpec& descriptions)
    {
      return create(Utils::Point(width, height), descriptions);
    }

    /** Adopts an unpopulated bottom layer and builds its reduced levels. */
    static Ptr create(const Layer::Ptr& bottom, const LayerSpec& descriptions);

    /** The level sequence remains valid for the lifetime of the stack. */
    [[nodiscard]] std::span<const Level> getLevels() const;

    void setSource(SourcePresentation::Ptr source);

    /** Starts a decoder callback and retains its cancellation registration. */
    void startLoading(const ReloadFunction& startLoading);

    /** Callbacks run on subscribing or population threads. Retain the returned subscription. */
    [[nodiscard]] Utils::Stuff subscribeProgress(const ProgressInterface::Ptr& progress);

    void tileCreated(const CompressedTile::Ptr& tile) override;
    void tileFinished(const CompressedTile::Ptr& tile) override;

  private:
    LayerStack() = default;
    void initialize(const Layer::Ptr& bottom, const LayerSpec& descriptions);
    void connect(const Layer::Ptr& layer, const Layer::Ptr& previousLayer, const LayerReduction& reducePreviousLayer);
  };
} // namespace Scroom::TiledBitmap
