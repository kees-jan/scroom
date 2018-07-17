/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "tiled-bitmap.hh"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

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
  const int pagesize = 4096;
  int pagesPerBlock = std::max(int(ceil(guessedTileSizeAfterCompression / 10 / pagesize)),1);

  int blockSize = pagesPerBlock*pagesize;
  int blockCount = std::max(int(ceil(tileCount / 10)), 64);

  printf("Creating a PageProvider providing %d blocks of %d bytes\n", blockCount, blockSize);
  return Scroom::MemoryBlobs::PageProvider::create(blockCount, blockSize);
}

////////////////////////////////////////////////////////////////////////

FileOperation::FileOperation(ProgressInterface::Ptr progress)
  : progress(progress), waitingMutex(), waiting(true)
{
  progress->setWaiting();
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

LoadOperation::LoadOperation(ThreadPool::WeakQueue::Ptr queue,
                             Layer::Ptr const& l, SourcePresentation::Ptr sp,
                             ProgressInterface::Ptr progress)
  : FileOperation(progress), target(l), thePresentation(sp), queue(queue)
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

TiledBitmap::TiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec const& ls)
  :bitmapWidth(bitmapWidth), bitmapHeight(bitmapHeight), ls(ls), tileCount(0), tileFinishedCount(0),
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

    Layer::Ptr layer = Layer::create(shared_from_this<TiledBitmap>(), i, width, height, bpp, provider);
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

  std::vector<LayerCoordinator::Ptr> coordinators;

  for(int j=0; j<verTileCount; j++)
  {
    int voffset = j%8;
    if(!voffset)
    {
      // New line of target tiles
      coordinators.clear();
      CompressedTileLine& tileLine = layer->getTileLine(j/8);
      for(unsigned int z=0; z<tileLine.size(); z++)
      {
        LayerCoordinator::Ptr lc = LayerCoordinator::create(tileLine[z], prevLo);
        coordinators.push_back(lc);
        this->coordinators.push_back(lc);
      }
    }

    for(int i=0; i<horTileCount; i++)
    {
      int hoffset = i%8;
      LayerCoordinator::Ptr lc = coordinators[i/8];
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

inline void computeAreasBeginningZoomingIn(int presentationBegin, int tileOffset, int pixelSize,
                                           int& tileBegin, int& viewBegin)
{
  if(tileOffset >= presentationBegin)
  {
    tileBegin = 0;
    viewBegin = (tileOffset-presentationBegin)*pixelSize;
  }
  else
  {
    tileBegin = presentationBegin-tileOffset;
    viewBegin = 0;
  }
}

inline void computeAreasEndZoomingIn(int presentationBegin, int presentationSize,
                            int tileOffset, int pixelSize,
                            int tileBegin, int& tileSize,
                            int viewBegin, int& viewSize)
{
  UNUSED(viewBegin);

  if(tileOffset + TILESIZE <= presentationBegin + presentationSize)
  {
    tileSize = TILESIZE - tileBegin;
  }
  else
  {
    tileSize = presentationBegin + presentationSize - tileOffset - tileBegin;
  }
  viewSize = tileSize*pixelSize;
}

inline void computeAreasBeginningZoomingOut(int presentationBegin, int tileOffset, int pixelSize,
                                           int& tileBegin, int& viewBegin)
{
  if(tileOffset >= presentationBegin)
  {
    tileBegin = 0;
    viewBegin = (tileOffset-presentationBegin)/pixelSize;
  }
  else
  {
    tileBegin = presentationBegin-tileOffset;
    viewBegin = 0;
  }
}

inline void computeAreasEndZoomingOut(int presentationBegin, int presentationSize,
                            int tileOffset, int pixelSize,
                            int tileBegin, int& tileSize,
                            int viewBegin, int& viewSize)
{
  UNUSED(viewBegin);

  if(tileOffset + TILESIZE <= presentationBegin + presentationSize)
  {
    tileSize = TILESIZE - tileBegin;
  }
  else
  {
    tileSize = presentationBegin + presentationSize - tileOffset - tileBegin;
  }
  viewSize = tileSize/pixelSize;
}

void TiledBitmap::drawTile(cairo_t* cr, const CompressedTile::Ptr tile, const GdkRectangle viewArea)
{
  cairo_set_source_rgb(cr, 0, 0, 0); // Black
  cairo_move_to(cr, viewArea.x, viewArea.y);
  cairo_line_to(cr, viewArea.x+viewArea.width, viewArea.y);
  cairo_line_to(cr, viewArea.x+viewArea.width, viewArea.y+viewArea.height);
  cairo_line_to(cr, viewArea.x, viewArea.y+viewArea.height);
  cairo_line_to(cr, viewArea.x, viewArea.y);
  cairo_stroke(cr);
  char buffer[256];
  snprintf(buffer, 256, "Layer %d, Tile (%d, %d), %d bpp",
           tile->depth, tile->x, tile->y, tile->bpp);
  cairo_move_to(cr, viewArea.x+20, viewArea.y+20);
  cairo_show_text(cr, buffer);

}

void TiledBitmap::redrawZoomingIn(ViewInterface::Ptr const& vi, cairo_t* cr, Rectangle<int> const& requestedPresentationArea, int zoom)
{
  TiledBitmapViewData::Ptr viewData = this->viewData[vi];
  GdkRectangle presentationArea = requestedPresentationArea.toGdkRectangle();

  Layer::Ptr layer = layers[0];
  LayerOperations::Ptr layerOperations = ls[0];

  const Rectangle<int> actualPresentationArea = layer->getRect();
  const Rectangle<int> validPresentationArea = requestedPresentationArea.intersection(actualPresentationArea);

  presentationArea.width = validPresentationArea.getRightPos() - requestedPresentationArea.getLeftPos();
  presentationArea.height = validPresentationArea.getBottomPos() - requestedPresentationArea.getTopPos();

  const int left = presentationArea.x;
  const int top = presentationArea.y;
  const int right = presentationArea.x+presentationArea.width;
  const int bottom = presentationArea.y+presentationArea.height;

  const int imin = std::max(0, left/TILESIZE);
  const int imax = (right+TILESIZE-1)/TILESIZE;
  const int jmin = std::max(0, top/TILESIZE);
  const int jmax = (bottom+TILESIZE-1)/TILESIZE;

  viewData->setNeededTiles(layer, imin, imax, jmin, jmax, zoom, layerOperations);

  const int pixelSize = 1<<zoom;

  layerOperations->initializeCairo(cr);

  for(int i=imin; i<imax; i++)
  {
    for(int j=jmin; j<jmax; j++)
    {
      GdkRectangle tileArea;
      GdkRectangle viewArea;

      computeAreasBeginningZoomingIn(presentationArea.x, i*TILESIZE, pixelSize, tileArea.x, viewArea.x);
      computeAreasBeginningZoomingIn(presentationArea.y, j*TILESIZE, pixelSize, tileArea.y, viewArea.y);
      computeAreasEndZoomingIn(presentationArea.x, presentationArea.width, i*TILESIZE, pixelSize,
                               tileArea.x, tileArea.width, viewArea.x, viewArea.width);
      computeAreasEndZoomingIn(presentationArea.y, presentationArea.height, j*TILESIZE, pixelSize,
                               tileArea.y, tileArea.height, viewArea.y, viewArea.height);

      CompressedTile::Ptr tile = layer->getTile(i,j);
      TileViewState::Ptr tileViewState = tile->getViewState(vi);
      Scroom::Utils::Stuff cacheResult = tileViewState->getCacheResult();
      ConstTile::Ptr t = tile->getConstTileAsync();

      if(t)
      {
        cairo_save(cr);
        layerOperations->draw(cr, t, tileArea, viewArea, zoom, cacheResult);
        cairo_restore(cr);
      }
      else
      {
        layerOperations->drawState(cr, tile->getState(), viewArea);
      }
#ifdef DEBUG_TILES
      drawTile(cr, tile, viewArea);
#endif
    }
  }
}

void TiledBitmap::redrawZoomingOut(ViewInterface::Ptr const& vi, cairo_t* cr, Rectangle<int> const& requestedPresentationArea, int zoom)
{
  TiledBitmapViewData::Ptr viewData = this->viewData[vi];
  Rectangle<int> scaledRequestedPresentationArea = requestedPresentationArea;

  // 1. Pick the correct layer
  unsigned int layerNr=0;
  while(zoom<=-3 && layerNr<layers.size()-1)
  {
    layerNr++;
    zoom+=3;
    scaledRequestedPresentationArea/=8;
  }
  Layer::Ptr layer = layers[layerNr];
  LayerOperations::Ptr layerOperations = ls[std::min(ls.size()-1, (size_t)layerNr)];

  const Rectangle<int> actualPresentationArea = layer->getRect();
  const Rectangle<int> validPresentationArea = scaledRequestedPresentationArea.intersection(actualPresentationArea);

  scaledRequestedPresentationArea.setSize
    (validPresentationArea.getBottomRight() - scaledRequestedPresentationArea.getTopLeft());

  const int left = scaledRequestedPresentationArea.getLeftPos();
  const int top = scaledRequestedPresentationArea.getTopPos();
  const int right = scaledRequestedPresentationArea.getRightPos();
  const int bottom = scaledRequestedPresentationArea.getBottomPos();

  const int imin = std::max(0, left/TILESIZE);
  const int imax = (right+TILESIZE-1)/TILESIZE;
  const int jmin = std::max(0, top/TILESIZE);
  const int jmax = (bottom+TILESIZE-1)/TILESIZE;

  viewData->setNeededTiles(layer, imin, imax, jmin, jmax, zoom, layerOperations);

  const int pixelSize = 1<<-zoom;

  layerOperations->initializeCairo(cr);

  GdkRectangle presentationArea = scaledRequestedPresentationArea.toGdkRectangle();
  for(int i=imin; i<imax; i++)
  {
    for(int j=jmin; j<jmax; j++)
    {

      GdkRectangle tileArea;
      GdkRectangle viewArea;

      // 2. Determine which area in the layer needs being drawn
      computeAreasBeginningZoomingOut(presentationArea.x, i*TILESIZE, pixelSize, tileArea.x, viewArea.x);
      computeAreasBeginningZoomingOut(presentationArea.y, j*TILESIZE, pixelSize, tileArea.y, viewArea.y);
      computeAreasEndZoomingOut(presentationArea.x, presentationArea.width, i*TILESIZE, pixelSize,
                                tileArea.x, tileArea.width, viewArea.x, viewArea.width);
      computeAreasEndZoomingOut(presentationArea.y, presentationArea.height, j*TILESIZE, pixelSize,
                                tileArea.y, tileArea.height, viewArea.y, viewArea.height);

      CompressedTile::Ptr tile = layer->getTile(i,j);
      TileViewState::Ptr tileViewState = tile->getViewState(vi);
      Scroom::Utils::Stuff cacheResult = tileViewState->getCacheResult();
      ConstTile::Ptr t = tile->getConstTileAsync();

      // 3. Draw the area
      if(t)
      {
        cairo_save(cr);
        layerOperations->draw(cr, t, tileArea, viewArea, zoom, cacheResult);
        cairo_restore(cr);
      }
      else
      {
        layerOperations->drawState(cr, tile->getState(), viewArea);
      }
#ifdef DEBUG_TILES
      drawTile(cr, tile, viewArea);
#endif
    }
  }
}

void TiledBitmap::redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Rectangle<double> const& presentationArea, int zoom)
{
  if(zoom>0)
    redrawZoomingIn(vi, cr, presentationArea.toIntRectangle(), zoom);
  else
    redrawZoomingOut(vi, cr, presentationArea.toIntRectangle(), zoom);
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
