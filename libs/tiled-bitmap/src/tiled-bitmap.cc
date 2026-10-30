/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "tiled-bitmap.hh"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cmath>
#include <cstdio>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <boost/thread/mutex.hpp>

#include <scroom/cairo-helpers.hh>
#include <scroom/semaphore.hh>
#include <scroom/tiled-bitmap-geometry.hh>

#include "scroom/tweak-view.hh"
#include "tileviewstate.hh"
#include "tweak-view.hh"

TiledBitmapInterface::Ptr createTiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec const& ls)
{
  return TiledBitmap::create(bitmapWidth, bitmapHeight, ls);
}

Scroom::Utils::Stuff Scroom::TiledBitmap::scheduleLoadingBitmap(
  const SourcePresentation::Ptr& sp,
  const Layer::Ptr& layer,
  const ProgressInterface::Ptr& progress
)
{
  auto wait_until_done = std::make_shared<Scroom::Semaphore>();
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
      Scroom::GtkHelpers::sync_on_ui_thread([=] { progress->setWorking(0); });
      layer->fetchData(sp, weakQueue, on_finished);
      wait_until_done->P();
    }
  );

  return Scroom::Utils::on_destruction(abort);
}

////////////////////////////////////////////////////////////////////////

namespace Geometry = Scroom::TiledBitmap::Geometry;

////////////////////////////////////////////////////////////////////////
// TiledBitmap

TiledBitmap::Ptr TiledBitmap::create(int bitmapWidth, int bitmapHeight, LayerSpec const& ls)
{
  TiledBitmap::Ptr result(new TiledBitmap(bitmapWidth, bitmapHeight, ls));
  result->initialize();
  return result;
}

TiledBitmap::Ptr TiledBitmap::create(const Layer::Ptr& bottom, LayerSpec const& ls)
{
  TiledBitmap::Ptr result(new TiledBitmap(bottom->getWidth(), bottom->getHeight(), ls));
  result->initialize(bottom);
  return result;
}

TiledBitmap::TiledBitmap(int bitmapWidth_, int bitmapHeight_, LayerSpec ls_)
  : bitmapWidth(bitmapWidth_)
  , bitmapHeight(bitmapHeight_)
  , ls(std::move(ls_))
  , progressBroadcaster(Scroom::Utils::ProgressInterfaceBroadcaster::create())
  , context(Scroom::Utils::Context::create())
{
  set(context, Scroom::TiledBitmap::PixelAlignPresentationPositions::create());
  set(context, Scroom::Utils::ParseAndFormatViewCentre::create());
  set(context, Scroom::Utils::CalculateViewAreaForRulers::create());
  set(
    context,
    Scroom::Utils::ITweakSelection::Map{
      {SelectionType::GRID, Scroom::TiledBitmap::SnapSelectionsToGrid::create()},
      {SelectionType::PIXEL, Scroom::TiledBitmap::ExpandSelectionsToPixelBoundaries::create()}
    }
  );
}

void TiledBitmap::initialize(const Layer::Ptr& bottom)
{
  const auto sizes = Geometry::computePyramidSizes({bitmapWidth, bitmapHeight}, TILESIZE / 4);
  LayerOperations::Ptr lo = ls[0];
  Scroom::MemoryBlobs::PageProvider::Ptr const provider = bottom->getPageProvider();

  registrations.emplace_back(bottom->registerObserver(shared_from_this<TileInitialisationObserver>()));
  layers.push_back(bottom);

  Layer::Ptr prevLayer = bottom;
  LayerOperations::Ptr prevLo = lo;

  for(unsigned int i = 1; i < sizes.size(); i++)
  {
    if(i < ls.size())
    {
      lo = ls[i];
    }

    Layer::Ptr const layer = Layer::create(i, sizes[i].x, sizes[i].y, lo->getBpp(), provider);
    registrations.emplace_back(layer->registerObserver(shared_from_this<TileInitialisationObserver>()));
    layers.push_back(layer);

    connect(layer, prevLayer, prevLo);

    prevLayer = layer;
    prevLo = lo;
  }
}

void TiledBitmap::initialize() { initialize(Layer::create(bitmapWidth, bitmapHeight, ls[0]->getBpp())); }

TiledBitmap::~TiledBitmap()
{
  logger->debug("TiledBitmap: Destructing...");

  coordinators.clear();
  layers.clear();
}

void TiledBitmap::connect(Layer::Ptr const& layer, Layer::Ptr const& prevLayer, const LayerOperations::Ptr& prevLo)
{
  const int horTileCount = prevLayer->getHorTileCount();
  const int verTileCount = prevLayer->getVerTileCount();

  std::vector<LayerCoordinator::Ptr> coordinators_;

  for(int j = 0; j < verTileCount; j++)
  {
    const auto rowMapping = Geometry::computeReductionMapping({0, j});
    const int voffset = rowMapping.offset.y;
    if(!voffset)
    {
      // New line of target tiles
      coordinators_.clear();
      CompressedTileLine const& tileLine = layer->getTileLine(rowMapping.targetTile.y);
      for(auto& z: tileLine)
      {
        LayerCoordinator::Ptr const lc = LayerCoordinator::create(z, prevLo);
        coordinators_.push_back(lc);
        coordinators.push_back(lc);
      }
    }

    for(int i = 0; i < horTileCount; i++)
    {
      const auto mapping = Geometry::computeReductionMapping({i, j});
      LayerCoordinator::Ptr const lc = coordinators_[mapping.targetTile.x];
      lc->addSourceTile(mapping.offset.x, mapping.offset.y, prevLayer->getTile(i, j));
    }
  }
}

////////////////////////////////////////////////////////////////////////
// TiledBitmapInterface

void TiledBitmap::setSource(SourcePresentation::Ptr sp)
{
  registrations.push_back(Scroom::TiledBitmap::scheduleLoadingBitmap(sp, layers[0], progressBroadcaster));
}

Layer::Ptr TiledBitmap::getBottomLayer() { return layers[0]; }

void TiledBitmap::drawTile(cairo_t* cr, const CompressedTile::Ptr& tile, const Scroom::Utils::Rectangle<double>& viewArea)
{
  const int margin = 5;

  if(viewArea.width() > 2 * margin && viewArea.height() > 2 * margin)
  {
    Scroom::Utils::Rectangle<double> const rect(
      viewArea.x() + margin, viewArea.y() + margin, viewArea.width() - 2 * margin, viewArea.height() - 2 * margin
    );

    cairo_set_source_rgb(cr, 0, 0, 0); // Black
    drawRectangleContour(cr, rect);

    const std::string label = fmt::format("Layer {}, Tile ({}, {}), {} bpp", tile->depth, tile->x, tile->y, tile->bpp);
    cairo_move_to(cr, rect.x() + 20, rect.y() + 20);
    cairo_show_text(cr, label.c_str());
  }
}

void TiledBitmap::redraw(
  ViewInterface::Ptr const& vi,
  cairo_t* cr,
  Scroom::Utils::Rectangle<double> const& presentationArea,
  int zoom
)
{
  TiledBitmapViewData::Ptr const viewData_ = viewData[vi];
  const auto selection = Geometry::computeLayerSelection(presentationArea, zoom, layers.size());
  const auto layerNr = selection.layer;
  zoom = selection.zoom;
  const auto scaledRequestedPresentationArea = selection.area;
  Layer::Ptr const layer = layers[layerNr];
  LayerOperations::Ptr const layerOperations = ls[std::min(ls.size() - 1, layerNr)];

  const auto tileRange = Geometry::computeTileRangeForRedraw(scaledRequestedPresentationArea, TILESIZE);

  viewData_->setNeededTiles(
    layer, tileRange.getLeft(), tileRange.getRight(), tileRange.getTop(), tileRange.getBottom(), zoom, layerOperations
  );

  const double pixelSize = pixelSizeFromZoom(zoom);

  layerOperations->initializeCairo(cr);

  const auto clippedRequestedPresentationArea =
    Geometry::computeClippedDrawArea(scaledRequestedPresentationArea, layer->getRect());

  for(int i = tileRange.getLeft(); i < tileRange.getRight(); i++)
  {
    for(int j = tileRange.getTop(); j < tileRange.getBottom(); j++)
    {
      Scroom::Utils::Point<int> const tileIndex(i, j);

      const auto areas = Geometry::computeTileDrawAreas(
        clippedRequestedPresentationArea, Geometry::computeTileRectangle(tileIndex, TILESIZE), pixelSize
      );

      CompressedTile::Ptr const tile = layer->getTile(i, j);
      TileViewState::Ptr const tileViewState = tile->getViewState(vi);
      Scroom::Utils::Stuff const cacheResult = tileViewState->getCacheResult();
      ConstTile::Ptr const t = tile->getConstTileAsync();

      if(t)
      {
        cairo_save(cr);
        layerOperations->draw(cr, t, areas.source, areas.destination, zoom, cacheResult);
        cairo_restore(cr);
      }
      else
      {
        layerOperations->drawState(cr, tile->getState(), areas.destination);
      }
#ifdef DEBUG_TILES
      drawTile(cr, tile, areas.destination);
#endif
    }
  }
}

void TiledBitmap::clearCaches(ViewInterface::Ptr viewInterface)
{
  boost::mutex::scoped_lock const lock(viewDataMutex);
  TiledBitmapViewData::Ptr const tbvd = viewData[viewInterface];
  if(tbvd)
  {
    tbvd->clearVolatileStuff();
  }
}

Scroom::Utils::Context::ConstPtr TiledBitmap::getContext() const { return context; }

void TiledBitmap::open(ViewInterface::WeakPtr viewInterface)
{
  boost::mutex::scoped_lock lock(viewDataMutex);
  TiledBitmapViewData::Ptr const vd = TiledBitmapViewData::create(viewInterface);
  viewData[viewInterface] = vd;
  vd->token.add(progressBroadcaster->subscribe(vd));
  lock.unlock();

  for(Layer::Ptr const& l: layers)
  {
    l->open(viewInterface);
  }
}

void TiledBitmap::close(ViewInterface::WeakPtr vi)
{
  for(Layer::Ptr const& l: layers)
  {
    l->close(vi);
  }

  boost::mutex::scoped_lock const lock(viewDataMutex);
  TiledBitmapViewData::Ptr const vd = viewData[vi];
  // Yuk. ProgressBroadcaster has a reference to viewData, so erasing it
  // from the map isn't enough.
  vd->token.reset();
  viewData.erase(vi);
}

////////////////////////////////////////////////////////////////////////
// TileInitialisationObserver

void TiledBitmap::tileCreated(const CompressedTile::Ptr& /*tile*/) { tileCount++; }

void TiledBitmap::tileFinished(const CompressedTile::Ptr& /*tile*/)
{
  boost::mutex::scoped_lock const lock(tileFinishedMutex);
  tileFinishedCount++;
  if(tileFinishedCount > tileCount)
  {
    defect_message("ERROR: Too many tiles are finished!");
  }
  else
  {
    Scroom::GtkHelpers::sync_on_ui_thread(
      [this]
      {
        progressBroadcaster->setWorking(1.0 * tileFinishedCount / tileCount);
        if(tileFinishedCount == tileCount)
        {
          progressBroadcaster->setFinished();
          logger->info("Finished loading file");
        }
      }
    );
  }
}
