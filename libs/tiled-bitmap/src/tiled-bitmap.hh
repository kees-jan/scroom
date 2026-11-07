/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>
#include <vector>

#include <boost/thread/mutex.hpp>

#include <scroom/interface.hh>
#include <scroom/layer-interface.hh>
#include <scroom/layer-stack.hh>
#include <scroom/scroominterface.hh>
#include <scroom/tiledbitmapinterface.hh>

#include "tiledbitmapviewdata.hh"

class TiledBitmap
  : public TiledBitmapInterface
  , public virtual Scroom::Utils::Base
{
public:
  using Ptr = std::shared_ptr<TiledBitmap>;
  using WeakPtr = std::weak_ptr<TiledBitmap>;
  using ViewDataMap = Scroom::Utils::WeakKeyMap<ViewInterface::WeakPtr, TiledBitmapViewData::Ptr>;

private:
  Scroom::TiledBitmap::LayerStack::Ptr m_stack;
  std::vector<LayerRenderingOperations::Ptr> m_rendering;
  boost::mutex viewDataMutex;
  ViewDataMap viewData;
  Scroom::Utils::Context::Ptr context;

public:
  static Ptr create(int bitmapWidth, int bitmapHeight, LayerSpec const& ls);
  static Ptr create(Scroom::TiledBitmap::LayerStack::Ptr stack, const LayerSpec& descriptions);

private:
  TiledBitmap(Scroom::TiledBitmap::LayerStack::Ptr stack, const LayerSpec& ls);

  static void drawTile(
    cairo_t* cr,
    std::size_t layerIndex,
    Scroom::Utils::Point<int> tileIndex,
    int bpp,
    const Scroom::Utils::Rectangle<double>& viewArea
  );

public:
  ////////////////////////////////////////////////////////////////////////
  // TiledBitmapInterface

public:
  void setSource(SourcePresentation::Ptr sp) override;
  LayerInterface::Ptr getBottomLayer() override;
  Scroom::TiledBitmap::LayerStack::Ptr getLayerStack() override;

  void open(ViewInterface::WeakPtr viewInterface) override;
  void close(ViewInterface::WeakPtr vi) override;
  void redraw(
    ViewInterface::Ptr const& vi,
    cairo_t* cr,
    Scroom::Utils::Rectangle<double> const& presentationArea,
    int zoom
  ) override;
  void clearCaches(ViewInterface::Ptr vi) override;

  [[nodiscard]] Scroom::Utils::Context::ConstPtr getContext() const override;
};
