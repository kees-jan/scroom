/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "tiled-bitmap.hh"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include <cmath>

#include <boost/thread/mutex.hpp>

#include <scroom/assertions.hh>
#include <scroom/cairo-helpers.hh>
#include <scroom/semaphore.hh>
#include <scroom/unused.hh>

#include "tileviewstate.hh"

TiledBitmapInterface::Ptr createTiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec const& ls)
{
  return TiledBitmap::create(bitmapWidth, bitmapHeight, ls);
}

////////////////////////////////////////////////////////////////////////
static Scroom::MemoryBlobs::PageProvider::Ptr createProvider(double width, double height, int bpp)
{
  double tileCount = (width*height) / (TILESIZE * TILESIZE);
  double tileSize = (bpp / 8.0) * TILESIZE * TILESIZE;

  double guessedTileSizeAfterCompression = tileSize / 100;
  const size_t pagesize = 4096;
  size_t pagesPerBlock = static_cast<size_t>(std::max(ceil(guessedTileSizeAfterCompression / 10.0 / static_cast<double>(pagesize)), 1.0));

  size_t blockSize = pagesPerBlock*pagesize;
  size_t blockCount = static_cast<size_t>(std::max(ceil(tileCount / 10.0), 64.0));

  printf("Creating a PageProvider providing %zu blocks of %zu bytes\n", blockCount, blockSize);
  return Scroom::MemoryBlobs::PageProvider::create(blockCount, blockSize);
}

////////////////////////////////////////////////////////////////////////

inline Scroom::Utils::Rectangle<int> TileAreaForIndex(Scroom::Utils::Point<int> tileIndex)
{
  return (Scroom::Utils::Rectangle<int>(0,0,1,1) + tileIndex) * TILESIZE;
}

////////////////////////////////////////////////////////////////////////

FileOperation::FileOperation(ProgressInterface::Ptr progress_)
  : progress(progress_), waitingMutex(), waiting(true)
{
  progress_->setWaiting();
}

void FileOperation::doneWaiting()
{
  boost::mutex::scoped_lock lock(waitingMutex);
  if(waiting)
  {
    gdk_threads_enter();
    progress->setWorking(0);
    gdk_threads_leave();
    waiting = false;
  }
}

////////////////////////////////////////////////////////////////////////

class LoadOperation : public FileOperation
{
private:
  Layer::Ptr target;
  SourcePresentation::Ptr thePresentation;
  Scroom::Semaphore done;
  ThreadPool::WeakQueue::Ptr queue;

private:
  LoadOperation(ThreadPool::WeakQueue::Ptr queue, Layer::Ptr const& l, SourcePresentation::Ptr sp,
                ProgressInterface::Ptr progress);
public:
  static Ptr create(ThreadPool::WeakQueue::Ptr queue, Layer::Ptr const& l, SourcePresentation::Ptr sp,
                    ProgressInterface::Ptr progress);

  virtual ~LoadOperation() {}

  virtual void operator()();
  virtual void finished();
  virtual void abort();
};

FileOperation::Ptr LoadOperation::create(ThreadPool::WeakQueue::Ptr queue,
                                         Layer::Ptr const& l, SourcePresentation::Ptr sp,
                                         ProgressInterface::Ptr progress)
{
  return FileOperation::Ptr(new LoadOperation(queue, l, sp, progress));
}

LoadOperation::LoadOperation(ThreadPool::WeakQueue::Ptr queue_,
                             Layer::Ptr const& l, SourcePresentation::Ptr sp,
                             ProgressInterface::Ptr progress_)
  : FileOperation(progress_), target(l), thePresentation(sp), queue(queue_)
{
}

void LoadOperation::operator()()
{
  doneWaiting();

  target->fetchData(thePresentation, queue);
  done.P();
}

void LoadOperation::abort()
{
  done.V();
}

void LoadOperation::finished()
{
  done.V();
}

////////////////////////////////////////////////////////////////////////
// TiledBitmap

TiledBitmap::Ptr TiledBitmap::create(int bitmapWidth, int bitmapHeight, LayerSpec const& ls)
{
  TiledBitmap::Ptr result(new TiledBitmap(bitmapWidth, bitmapHeight, ls));
  result->initialize();
  return result;
}

TiledBitmap::TiledBitmap(int bitmapWidth_, int bitmapHeight_, LayerSpec const& ls_)
  :bitmapWidth(bitmapWidth_), bitmapHeight(bitmapHeight_), ls(ls_), tileCount(0), tileFinishedCount(0),
   fileOperation(), progressBroadcaster(Scroom::Utils::ProgressInterfaceBroadcaster::create()), queue(ThreadPool::Queue::createAsync())
{
}

void TiledBitmap::initialize()
{
  int width = bitmapWidth;
  int height = bitmapHeight;
  unsigned int i = 0;
  int bpp = 0;
  LayerOperations::Ptr lo = ls[i];
  Layer::Ptr prevLayer;
  LayerOperations::Ptr prevLo;
  Scroom::MemoryBlobs::PageProvider::Ptr provider = createProvider(width, height, lo->getBpp());
  do
  {
    if(i<ls.size())
      lo = ls[i];

    bpp = lo->getBpp();

    Layer::Ptr layer = Layer::create(shared_from_this<TiledBitmap>(), static_cast<int>(i), width, height, bpp, provider);
    layers.push_back(layer);
    if(prevLayer)
    {
      connect(layer, prevLayer, prevLo);
    }

    prevLayer = layer;
    prevLo = lo;
    width = (width+7)/8; // Round up
    height = (height+7)/8;
    i++;
  } while (std::max(width, height) > TILESIZE/4);
}

TiledBitmap::~TiledBitmap()
{
  printf("TiledBitmap: Destructing...\n");
  queue.reset();

  if(fileOperation)
  {
    fileOperation->abort();
    fileOperation.reset();
  }
  coordinators.clear();
  layers.clear();
}

void TiledBitmap::connect(Layer::Ptr const& layer, Layer::Ptr const& prevLayer,
                          LayerOperations::Ptr prevLo)
{
  int horTileCount = prevLayer->getHorTileCount();
  int verTileCount = prevLayer->getVerTileCount();

  std::vector<LayerCoordinator::Ptr> coordinators_;

  for(int j=0; j<verTileCount; j++)
  {
    int voffset = j%8;
    if(!voffset)
    {
      // New line of target tiles
      coordinators_.clear();
      CompressedTileLine& tileLine = layer->getTileLine(j/8);
      for(unsigned int z=0; z<tileLine.size(); z++)
      {
        LayerCoordinator::Ptr lc = LayerCoordinator::create(tileLine[z], prevLo);
        coordinators_.push_back(lc);
        this->coordinators.push_back(lc);
      }
    }

    for(int i=0; i<horTileCount; i++)
    {
      int hoffset = i%8;
      LayerCoordinator::Ptr lc = coordinators_[static_cast<size_t>(i/8)];
      lc->addSourceTile(hoffset, voffset, prevLayer->getTile(i,j));
    }
  }
}

////////////////////////////////////////////////////////////////////////
// TiledBitmapInterface

void TiledBitmap::setSource(SourcePresentation::Ptr sp)
{
  if(!fileOperation)
  {
    fileOperation = LoadOperation::create(queue->getWeak(), layers[0], sp, progressBroadcaster);
    Sequentially()->schedule(fileOperation);
  }
  else
  {
    printf("PANIC: Another operation is already in progress\n");
  }
}

Layer::Ptr TiledBitmap::getBottomLayer()
{
  return layers[0];
}

void TiledBitmap::drawTile(cairo_t* cr, const CompressedTile::Ptr tile, const Scroom::Utils::Rectangle<double> viewArea)
{
  const int margin=5;

  if(viewArea.width()>2*margin && viewArea.height()>2*margin)
  {
    Scroom::Utils::Rectangle<double> rect(viewArea.x()+margin, viewArea.y()+margin, viewArea.width()-2*margin, viewArea.height()-2*margin);
  
    cairo_set_source_rgb(cr, 0, 0, 0); // Black
    drawRectangleContour(cr, rect);
    char buffer[256];
    snprintf(buffer, 256, "Layer %d, Tile (%d, %d), %d bpp",
             tile->depth, tile->x, tile->y, tile->bpp);
    cairo_move_to(cr, rect.x()+20, rect.y()+20);
    cairo_show_text(cr, buffer);
  }
}

void TiledBitmap::redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> const& presentationArea, int zoom)
{
  TiledBitmapViewData::Ptr viewData_ = this->viewData[vi];
  auto scaledRequestedPresentationArea = presentationArea;

  unsigned int layerNr=0;
  while(zoom<=-3 && layerNr<layers.size()-1)
  {
    layerNr++;
    zoom+=3;
    scaledRequestedPresentationArea/=8;
  }
  Layer::Ptr layer = layers[layerNr];
  LayerOperations::Ptr layerOperations = ls[std::min(ls.size()-1, static_cast<size_t>(layerNr))];

  const Scroom::Utils::Rectangle<int> actualPresentationArea = layer->getRect();
  const auto validPresentationArea = scaledRequestedPresentationArea.intersection(actualPresentationArea);

  const int left = static_cast<int>(scaledRequestedPresentationArea.getLeft());
  const int top = static_cast<int>(scaledRequestedPresentationArea.getTop());
  const int right = static_cast<int>(scaledRequestedPresentationArea.getRight());
  const int bottom = static_cast<int>(scaledRequestedPresentationArea.getBottom());

  const int imin = std::max(0, left/TILESIZE);
  const int imax = (right+TILESIZE-1)/TILESIZE;
  const int jmin = std::max(0, top/TILESIZE);
  const int jmax = (bottom+TILESIZE-1)/TILESIZE;

  viewData_->setNeededTiles(layer, imin, imax, jmin, jmax, zoom, layerOperations);

  const double pixelSize = pixelSizeFromZoom(zoom);

  layerOperations->initializeCairo(cr);

  const auto clippedRequestedPresentationArea = scaledRequestedPresentationArea
    .above(validPresentationArea.getBottom())
    .leftOf(validPresentationArea.getRight());
           
  for(int i=imin; i<imax; i++)
  {
    for(int j=jmin; j<jmax; j++)
    {
      Scroom::Utils::Point<int> tileIndex(i,j);

      const auto tileArea = TileAreaForIndex(tileIndex);
      const auto visibleTileArea = tileArea.intersection(clippedRequestedPresentationArea);

      const auto tileAreaRect = visibleTileArea - tileArea.getTopLeft();
      const auto viewAreaRect = (visibleTileArea - clippedRequestedPresentationArea.getTopLeft()) * pixelSize;

      CompressedTile::Ptr tile = layer->getTile(i,j);
      TileViewState::Ptr tileViewState = tile->getViewState(vi);
      Scroom::Utils::Stuff cacheResult = tileViewState->getCacheResult();
      ConstTile::Ptr t = tile->getConstTileAsync();

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
  TiledBitmapViewData::Ptr tbvd = viewData[viewInterface];
  if(tbvd)
  {
    tbvd->clearVolatileStuff();
  }
}

void TiledBitmap::open(ViewInterface::WeakPtr viewInterface)
{
  boost::mutex::scoped_lock lock(viewDataMutex);
  TiledBitmapViewData::Ptr vd = TiledBitmapViewData::create(viewInterface);
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

  boost::mutex::scoped_lock lock(viewDataMutex);
  TiledBitmapViewData::Ptr vd = viewData[vi];
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
  if(tileFinishedCount>tileCount)
  {
    printf("ERROR: Too many tiles are finished!\n");
  }
  else
  {
    gdk_threads_enter();
    progressBroadcaster->setWorking(1.0*tileFinishedCount/tileCount);
    if(tileFinishedCount==tileCount)
    {
      progressBroadcaster->setFinished();
      if(fileOperation)
      {
        fileOperation->finished();
        fileOperation.reset();
      }
      printf("INFO: Finished loading file\n");
    }
    gdk_threads_leave();
  }
}
