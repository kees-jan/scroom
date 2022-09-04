/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include <scroom/interface.hh>
#include <scroom/opentiledbitmapinterface.hh>
#include <scroom/progressinterfacehelpers.hh>
#include <scroom/scroominterface.hh>
#include <scroom/threadpool.hh>
#include <scroom/tiledbitmapinterface.hh>
#include <scroom/tiledbitmaplayer.hh>

#include "layercoordinator.hh"
#include "tiledbitmapviewdata.hh"

class TiledBitmap
  : public TiledBitmapInterface
  , public TileInitialisationObserver
  , public virtual Scroom::Utils::Base
{
public:
  using Ptr         = boost::shared_ptr<TiledBitmap>;
  using WeakPtr     = boost::weak_ptr<TiledBitmap>;
  using ViewDataMap = std::map<ViewInterface::WeakPtr, TiledBitmapViewData::Ptr>;

private:
  int                                              bitmapWidth;
  int                                              bitmapHeight;
  LayerSpec                                        ls;
  std::vector<Layer::Ptr>                          layers;
  std::list<LayerCoordinator::Ptr>                 coordinators;
  boost::mutex                                     viewDataMutex;
  ViewDataMap                                      viewData;
  int                                              tileCount{0};
  boost::mutex                                     tileFinishedMutex;
  int                                              tileFinishedCount{0};
  Scroom::Utils::ProgressInterfaceBroadcaster::Ptr progressBroadcaster;
  Scroom::Utils::StuffList                         registrations;

public:
  static Ptr create(int bitmapWidth, int bitmapHeight, LayerSpec const& ls);
  static Ptr create(const Layer::Ptr& bottom, const LayerSpec& ls);

  ~TiledBitmap() override;
  TiledBitmap(const TiledBitmap&)           = delete;
  TiledBitmap(TiledBitmap&&)                = delete;
  TiledBitmap operator=(const TiledBitmap&) = delete;
  TiledBitmap operator=(TiledBitmap&&)      = delete;

private:
  TiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec ls);

  void initialize();
  void initialize(const Layer::Ptr& bottom);

private:
  static void drawTile(cairo_t* cr, const CompressedTile::Ptr& tile, const Scroom::Utils::Rectangle<double>& viewArea);
  void        connect(Layer::Ptr const& layer, Layer::Ptr const& prevLayer, const LayerOperations::Ptr& prevLo);

public:
  ////////////////////////////////////////////////////////////////////////
  // TiledBitmapInterface

public:
  void       setSource(SourcePresentation::Ptr sp) override;
  Layer::Ptr getBottomLayer() override;

  void open(ViewInterface::WeakPtr viewInterface) override;
  void close(ViewInterface::WeakPtr vi) override;
  void redraw(ViewInterface::Ptr const&               vi,
              cairo_t*                                cr,
              Scroom::Utils::Rectangle<double> const& presentationArea,
              int                                     zoom) override;
  void clearCaches(ViewInterface::Ptr vi) override;

  ////////////////////////////////////////////////////////////////////////
  // TileInitialisationObserver

  void tileCreated(const CompressedTile::Ptr& tile) override;
  void tileFinished(const CompressedTile::Ptr& tile) override;

  ////////////////////////////////////////////////////////////////////////
  // Helpers
  ProgressInterface::Ptr progressInterface() { return progressBroadcaster; }
};
