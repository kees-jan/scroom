/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
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
#include <scroom/unused.hh>

#include "tileviewstate.hh"

TiledBitmapInterface::Ptr createTiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec const& ls)
{
  return TiledBitmap::create(bitmapWidth, bitmapHeight, ls);
}

Scroom::Utils::Stuff Scroom::TiledBitmap::scheduleLoadingBitmap(const SourcePresentation::Ptr& sp,
                                                                const Layer::Ptr&              layer,
                                                                const ProgressInterface::Ptr&  progress)
{
  auto wait_until_done = boost::make_shared<Scroom::Semaphore>();
  auto queue           = ThreadPool::Queue::createAsync();
  auto weakQueue       = queue->getWeak();
  auto abort           = [wait_until_done, queue]() mutable
  {
    queue.reset();
    wait_until_done->V();
  };

  auto on_finished = [wait_until_done] { wait_until_done->V(); };
  progress->setWaiting();

  Sequentially()->schedule(
    [progress, layer, sp, weakQueue, on_finished, wait_until_done]
    {
      Scroom::GtkHelpers::sync_on_ui_thread([=] { progress->setWorking(0); });
      layer->fetchData(sp, weakQueue, on_finished);
      wait_until_done->P();
    });

  return Scroom::Utils::on_destruction(abort);
}

////////////////////////////////////////////////////////////////////////

inline Scroom::Utils::Rectangle<int> TileAreaForIndex(Scroom::Utils::Point<int> tileIndex)
{
  return (Scroom::Utils::Rectangle<int>(0, 0, 1, 1) + tileIndex) * TILESIZE;
}

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
{
}

void TiledBitmap::initialize(const Layer::Ptr& bottom)
{
  int                                    width    = bitmapWidth;
  int                                    height   = bitmapHeight;
  unsigned int                           i        = 0;
  LayerOperations::Ptr                   lo       = ls[i];
  Scroom::MemoryBlobs::PageProvider::Ptr provider = bottom->getPageProvider();

  registrations.emplace_back(bottom->registerObserver(shared_from_this<TileInitialisationObserver>()));
  layers.push_back(bottom);

  Layer::Ptr           prevLayer = bottom;
  LayerOperations::Ptr prevLo    = lo;

  width  = (width + 7) / 8; // Round up
  height = (height + 7) / 8;
  i++;

  while(std::max(width, height) > TILESIZE / 4)
  {
    if(i < ls.size())
    {
      lo = ls[i];
    }

    Layer::Ptr layer = Layer::create(i, width, height, lo->getBpp(), provider);
    registrations.emplace_back(layer->registerObserver(shared_from_this<TileInitialisationObserver>()));
    layers.push_back(layer);

    connect(layer, prevLayer, prevLo);

    prevLayer = layer;
    prevLo    = lo;
    width     = (width + 7) / 8; // Round up
    height    = (height + 7) / 8;
    i++;
  }
}

void TiledBitmap::initialize() { initialize(Layer::create(bitmapWidth, bitmapHeight, ls[0]->getBpp())); }

TiledBitmap::~TiledBitmap()
{
  spdlog::debug("TiledBitmap: Destructing...");

  coordinators.clear();
  layers.clear();
}

void TiledBitmap::connect(Layer::Ptr const& layer, Layer::Ptr const& prevLayer, const LayerOperations::Ptr& prevLo)
{
  int horTileCount = prevLayer->getHorTileCount();
  int verTileCount = prevLayer->getVerTileCount();

  std::vector<LayerCoordinator::Ptr> coordinators_;

  for(int j = 0; j < verTileCount; j++)
  {
    int voffset = j % 8;
    if(!voffset)
    {
      // New line of target tiles
      coordinators_.clear();
      CompressedTileLine& tileLine = layer->getTileLine(j / 8);
      for(auto& z: tileLine)
      {
        LayerCoordinator::Ptr lc = LayerCoordinator::create(z, prevLo);
        coordinators_.push_back(lc);
        coordinators.push_back(lc);
      }
    }

    for(int i = 0; i < horTileCount; i++)
    {
      int                   hoffset = i % 8;
      LayerCoordinator::Ptr lc      = coordinators_[i / 8];
      lc->addSourceTile(hoffset, voffset, prevLayer->getTile(i, j));
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
    Scroom::Utils::Rectangle<double> rect(
      viewArea.x() + margin, viewArea.y() + margin, viewArea.width() - 2 * margin, viewArea.height() - 2 * margin);

    cairo_set_source_rgb(cr, 0, 0, 0); // Black
    drawRectangleContour(cr, rect);

    std::string label = fmt::format("Layer {}, Tile ({}, {}), {} bpp", tile->depth, tile->x, tile->y, tile->bpp);
    cairo_move_to(cr, rect.x() + 20, rect.y() + 20);
    cairo_show_text(cr, label.c_str());
  }
}

void TiledBitmap::redraw(ViewInterface::Ptr const&               vi,
                         cairo_t*                                cr,
                         Scroom::Utils::Rectangle<double> const& presentationArea,
                         int                                     zoom)
{
  TiledBitmapViewData::Ptr viewData_                       = viewData[vi];
  auto                     scaledRequestedPresentationArea = presentationArea;

  unsigned int layerNr = 0;
  while(zoom <= -3 && layerNr < layers.size() - 1)
  {
    layerNr++;
    zoom += 3;
    scaledRequestedPresentationArea /= 8;
  }
  Layer::Ptr           layer           = layers[layerNr];
  LayerOperations::Ptr layerOperations = ls[std::min(ls.size() - 1, static_cast<size_t>(layerNr))];

  const Scroom::Utils::Rectangle<int> actualPresentationArea = layer->getRect();
  const auto validPresentationArea = scaledRequestedPresentationArea.intersection(actualPresentationArea);

  const int left   = scaledRequestedPresentationArea.getLeft();
  const int top    = scaledRequestedPresentationArea.getTop();
  const int right  = scaledRequestedPresentationArea.getRight();
  const int bottom = scaledRequestedPresentationArea.getBottom();

  const int imin = std::max(0, left / TILESIZE);
  const int imax = (right + TILESIZE - 1) / TILESIZE;
  const int jmin = std::max(0, top / TILESIZE);
  const int jmax = (bottom + TILESIZE - 1) / TILESIZE;

  viewData_->setNeededTiles(layer, imin, imax, jmin, jmax, zoom, layerOperations);

  const double pixelSize = pixelSizeFromZoom(zoom);

  layerOperations->initializeCairo(cr);

  const auto clippedRequestedPresentationArea =
    scaledRequestedPresentationArea.above(validPresentationArea.getBottom()).leftOf(validPresentationArea.getRight());

  for(int i = imin; i < imax; i++)
  {
    for(int j = jmin; j < jmax; j++)
    {
      Scroom::Utils::Point<int> tileIndex(i, j);

      const auto tileArea        = TileAreaForIndex(tileIndex);
      const auto visibleTileArea = tileArea.intersection(clippedRequestedPresentationArea);

      const auto tileAreaRect = visibleTileArea - tileArea.getTopLeft();
      const auto viewAreaRect = (visibleTileArea - clippedRequestedPresentationArea.getTopLeft()) * pixelSize;

      CompressedTile::Ptr  tile          = layer->getTile(i, j);
      TileViewState::Ptr   tileViewState = tile->getViewState(vi);
      Scroom::Utils::Stuff cacheResult   = tileViewState->getCacheResult();
      ConstTile::Ptr       t             = tile->getConstTileAsync();

      if(t)
      {
        cairo_save(cr);
        layerOperations->draw(cr, t, tileAreaRect, viewAreaRect, zoom, cacheResult);
        cairo_restore(cr);
      }
      else
      {
        layerOperations->drawState(cr, tile->getState(), viewAreaRect);
      }
#ifdef DEBUG_TILES
      drawTile(cr, tile, viewAreaRect);
#endif
    }
  }
}

void TiledBitmap::clearCaches(ViewInterface::Ptr viewInterface)
{
  boost::mutex::scoped_lock lock(viewDataMutex);
  TiledBitmapViewData::Ptr  tbvd = viewData[viewInterface];
  if(tbvd)
  {
    tbvd->clearVolatileStuff();
  }
}

void TiledBitmap::open(ViewInterface::WeakPtr viewInterface)
{
  boost::mutex::scoped_lock lock(viewDataMutex);
  TiledBitmapViewData::Ptr  vd = TiledBitmapViewData::create(viewInterface);
  viewData[viewInterface]      = vd;
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

  boost::mutex::scoped_lock lock(viewDataMutex);
  TiledBitmapViewData::Ptr  vd = viewData[vi];
  // Yuk. ProgressBroadcaster has a reference to viewData, so erasing it
  // from the map isn't enough.
  vd->token.reset();
  viewData.erase(vi);
}

////////////////////////////////////////////////////////////////////////
// TileInitialisationObserver

void TiledBitmap::tileCreated(CompressedTile::Ptr tile)
{
  UNUSED(tile);
  tileCount++;
}

void TiledBitmap::tileFinished(CompressedTile::Ptr tile)
{
  UNUSED(tile);
  boost::mutex::scoped_lock lock(tileFinishedMutex);
  tileFinishedCount++;
  if(tileFinishedCount > tileCount)
  {
    defect_message("ERROR: Too many tiles are finished!");
  }
  else
  {
    Scroom::GtkHelpers::sync_on_ui_thread(
      [=]
      {
        progressBroadcaster->setWorking(1.0 * tileFinishedCount / tileCount);
        if(tileFinishedCount == tileCount)
        {
          progressBroadcaster->setFinished();
          spdlog::info("Finished loading file");
        }
      });
  }
}
