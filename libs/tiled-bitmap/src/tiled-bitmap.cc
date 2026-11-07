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

#include <algorithm>
#include <utility>

#include <fmt/format.h>

#include <boost/thread/mutex.hpp>

#include <scroom/assertions.hh>
#include <scroom/cairo-helpers.hh>
#include <scroom/gtk-helpers.hh>
#include <scroom/progressinterfacehelpers.hh>
#include <scroom/tiled-bitmap-geometry.hh>
#include <scroom/tiledbitmaplayer.hh>

#include "scroom/tweak-view.hh"
#include "tileviewstate.hh"
#include "tweak-view.hh"

namespace Geometry = Scroom::TiledBitmap::Geometry;

namespace
{
  class ForwardProgressToUiThread : public Scroom::Utils::ProgressInterfaceFromProgressStateInterface
  {
  public:
    explicit ForwardProgressToUiThread(const ProgressInterface::Ptr& child)
      : m_child(Scroom::Utils::ProgressStateInterfaceFromProgressInterfaceForwarder::create(child))
    {
    }

  private:
    Scroom::Utils::ProgressStateInterface::Ptr m_child;

    void setProgress(State state, double progress) override
    {
      // Queued updates must not retain a closed view.
      const Scroom::Utils::ProgressStateInterface::WeakPtr child = m_child;
      Scroom::GtkHelpers::async_on_ui_thread(
        [child, state, progress]
        {
          if(const auto target = child.lock())
          {
            target->setProgress(state, progress);
          }
        }
      );
    }
  };
} // namespace

TiledBitmapInterface::Ptr createTiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec const& ls)
{
  return TiledBitmap::create(bitmapWidth, bitmapHeight, ls);
}

TiledBitmapInterface::Ptr createTiledBitmap(const Scroom::TiledBitmap::LayerStack::Ptr& stack, const LayerSpec& descriptions)
{
  return TiledBitmap::create(stack, descriptions);
}

TiledBitmap::Ptr TiledBitmap::create(Scroom::TiledBitmap::LayerStack::Ptr stack, const LayerSpec& descriptions)
{
  require(stack);
  require(!descriptions.empty());
  return Ptr(new TiledBitmap(std::move(stack), descriptions));
}

TiledBitmap::Ptr TiledBitmap::create(int bitmapWidth, int bitmapHeight, const LayerSpec& ls)
{
  require(!ls.empty());
  const auto provider = createCompressedLayerPageProvider({bitmapWidth, bitmapHeight}, ls.front().bpp);
  return Ptr(new TiledBitmap(
    Scroom::TiledBitmap::LayerStack::create(bitmapWidth, bitmapHeight, ls, createCompressedLayerFactory(provider)), ls
  ));
}

TiledBitmap::TiledBitmap(Scroom::TiledBitmap::LayerStack::Ptr stack, const LayerSpec& ls)
  : m_stack(std::move(stack))
  , context(Scroom::Utils::Context::create())
{
  for(const auto& description: ls)
  {
    m_rendering.push_back(description.rendering);
  }
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


void TiledBitmap::setSource(SourcePresentation::Ptr source) { m_stack->setSource(std::move(source)); }
LayerInterface::Ptr TiledBitmap::getBottomLayer() { return m_stack->getLevels().front().layer; }
Scroom::TiledBitmap::LayerStack::Ptr TiledBitmap::getLayerStack() { return m_stack; }

void TiledBitmap::drawTile(
  cairo_t* cr,
  std::size_t layerIndex,
  Scroom::Utils::Point<int> tileIndex,
  int bpp,
  const Scroom::Utils::Rectangle<double>& viewArea
)
{
  const int margin = 5;

  if(viewArea.width() > 2 * margin && viewArea.height() > 2 * margin)
  {
    Scroom::Utils::Rectangle<double> const rect(
      viewArea.x() + margin, viewArea.y() + margin, viewArea.width() - 2 * margin, viewArea.height() - 2 * margin
    );

    cairo_set_source_rgb(cr, 0, 0, 0); // Black
    drawRectangleContour(cr, rect);

    const std::string label = fmt::format("Layer {}, Tile ({}, {}), {} bpp", layerIndex, tileIndex.x, tileIndex.y, bpp);
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
  const auto layerSelection = Geometry::layerSelection(presentationArea, zoom, m_stack->getLevels().size());
  const auto layerNr = layerSelection.index;
  const int layerZoom = layerSelection.zoom;
  const auto presentationAreaInLayerCoordinates = layerSelection.presentationAreaInLayerCoordinates;
  LayerInterface::Ptr const layer = m_stack->getLevels()[layerNr].layer;
  LayerRenderingOperations::Ptr const layerOperations = m_rendering[std::min(m_rendering.size() - 1, layerNr)];

  const auto tileRange = Geometry::tileRangeForRedraw(presentationAreaInLayerCoordinates, TILESIZE);

  viewData_->setNeededTiles(layer, tileRange, layerZoom, layerOperations);

  const double layerPixelSize = pixelSizeFromZoom(layerZoom);

  layerOperations->initializeCairo(cr);

  const auto clippedPresentationAreaInLayerCoordinates =
    Geometry::clippedArea(presentationAreaInLayerCoordinates, layer->getRect());

  for(const auto tileIndex: tileRange)
  {
    const auto drawAreas = Geometry::tileDrawAreas(
      clippedPresentationAreaInLayerCoordinates, Geometry::tileAreaInLayerCoordinates(tileIndex, TILESIZE), layerPixelSize
    );

    TileStorage::Ptr const tile = layer->getTile(tileIndex);
    TileViewState::Ptr const tileViewState = tile->getViewState(vi);
    Scroom::Utils::Stuff const cacheResult = tileViewState->getCacheResult();
    ConstTile::Ptr const t = tile->getConstTileAsync();

    if(t)
    {
      cairo_save(cr);
      layerOperations->draw(cr, t, drawAreas.tileArea, drawAreas.viewArea, layerZoom, cacheResult);
      cairo_restore(cr);
    }
    else
    {
      layerOperations->drawState(cr, tile->getState(), drawAreas.viewArea);
    }
#ifdef DEBUG_TILES
    drawTile(cr, layerNr, tileIndex, layer->getBpp(), drawAreas.viewArea);
#endif
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
  vd->token.add(m_stack->subscribeProgress(std::make_shared<ForwardProgressToUiThread>(vd)));
  lock.unlock();

  for(const auto& level: m_stack->getLevels())
  {
    level.layer->open(viewInterface);
  }
}

void TiledBitmap::close(ViewInterface::WeakPtr vi)
{
  for(const auto& level: m_stack->getLevels())
  {
    level.layer->close(vi);
  }

  boost::mutex::scoped_lock const lock(viewDataMutex);
  TiledBitmapViewData::Ptr const vd = viewData[vi];
  // Release the progress subscription before removing the view data.
  vd->token.reset();
  viewData.erase(vi);
}
