/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "tiled-bitmap.hh"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include <boost/thread/mutex.hpp>

#include <scroom/semaphore.hh>
#include <scroom/unused.h>

TiledBitmapInterface::Ptr createTiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls)
{
  return TiledBitmap::create(bitmapWidth, bitmapHeight, ls);
}

////////////////////////////////////////////////////////////////////////

gboolean timerExpired(gpointer data)
{
  return ((FileOperation*)data)->timerExpired();
}

FileOperation::FileOperation(TiledBitmap::Ptr parent)
  : parent(parent), waitingMutex(), waiting(true)
{
  timer = gtk_timeout_add(100, ::timerExpired, this);
}

void FileOperation::doneWaiting()
{
  boost::mutex::scoped_lock lock(waitingMutex);
  if(waiting)
  {
    gtk_timeout_remove(timer);
    waiting = false;
  }
}

bool FileOperation::timerExpired()
{
  boost::mutex::scoped_lock lock(waitingMutex);
  if(waiting)
    parent->gtk_progress_bar_pulse();

  return waiting;
}

////////////////////////////////////////////////////////////////////////

class LoadOperation : public FileOperation
{
private:
  Layer* target;
  SourcePresentation* thePresentation;
  Scroom::Semaphore done;

private:
  LoadOperation(Layer* l, SourcePresentation* sp, TiledBitmap::Ptr parent);
public:
  static Ptr create(Layer* l, SourcePresentation* sp, TiledBitmap::Ptr parent);
  
  virtual ~LoadOperation() {}

  virtual void operator()();
  virtual void finished();
};

FileOperation::Ptr LoadOperation::create(Layer* l, SourcePresentation* sp, TiledBitmap::Ptr parent)
{
  return FileOperation::Ptr(new LoadOperation(l, sp, parent));
}
                                         
LoadOperation::LoadOperation(Layer* l, SourcePresentation* sp, TiledBitmap::Ptr parent)
  : FileOperation(parent), target(l), thePresentation(sp)
{
}

void LoadOperation::operator()()
{
  doneWaiting();

  target->fetchData(thePresentation);
  done.P();
}

void LoadOperation::finished()
{
  done.V();
}

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// TiledBitmap

TiledBitmap::Ptr TiledBitmap::create(int bitmapWidth, int bitmapHeight, LayerSpec& ls)
{
  TiledBitmap::Ptr result(new TiledBitmap(bitmapWidth, bitmapHeight, ls));
  result->initialize();
  return result;
}

TiledBitmap::TiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls)
  :bitmapWidth(bitmapWidth), bitmapHeight(bitmapHeight), ls(ls), progressBar(NULL), tileCount(0), tileFinishedCount(0),
   fileOperation()
{
}

void TiledBitmap::initialize()
{
  int width = bitmapWidth;
  int height = bitmapHeight;
  unsigned int i = 0;
  int bpp = 0;
  LayerOperations* lo = NULL;
  Layer* prevLayer = NULL;
  LayerOperations* prevLo = NULL;
  do
  {
    if(i<ls.size())
      lo = ls[i];
    
    bpp = lo->getBpp();

    Layer* layer = new Layer(shared_from_this(), i, width, height, bpp);
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
  } while (std::max(width, height) > TILESIZE);
}

TiledBitmap::~TiledBitmap()
{
  printf("TiledBitmap: Destructing...\n");
  coordinators.clear();
  while(!layers.empty())
  {
    delete layers.back();
    layers.pop_back();
  }
}

void TiledBitmap::connect(Layer* layer, Layer* prevLayer,
                          LayerOperations* prevLo)
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
      TileInternalLine& til = layer->getTileLine(j/8);
      for(unsigned int z=0; z<til.size(); z++)
      {
        LayerCoordinator::Ptr lc = LayerCoordinator::create(til[z], prevLo);
        coordinators.push_back(lc);
        this->coordinators.push_back(lc);
      }
    }

    LayerCoordinator::Ptr lc;
    
    for(int i=0; i<horTileCount; i++)
    {
      int hoffset = i%8;
      if(!hoffset)
      {
        // New target tile
        lc = coordinators[i/8];
      }

      lc->addSourceTile(hoffset, voffset, prevLayer->getTile(i,j));
    }
  }
}

void TiledBitmap::gtk_progress_bar_set_fraction(double fraction)
{
  gdk_threads_enter();
  boost::mutex::scoped_lock lock(viewDataMutex);
  std::map<ViewInterface*, TiledBitmapViewData::Ptr>::iterator cur = viewData.begin();
  std::map<ViewInterface*, TiledBitmapViewData::Ptr>::iterator end = viewData.end();

  for(; cur!=end; ++cur)
  {
    // EEK! This is not thread safe!
    cur->second->gtk_progress_bar_set_fraction(fraction);
  }
  gdk_threads_leave();
}

void TiledBitmap::gtk_progress_bar_pulse()
{
  gdk_threads_enter();
  boost::mutex::scoped_lock lock(viewDataMutex);
  std::map<ViewInterface*, TiledBitmapViewData::Ptr>::iterator cur = viewData.begin();
  std::map<ViewInterface*, TiledBitmapViewData::Ptr>::iterator end = viewData.end();

  for(; cur!=end; ++cur)
  {
    // EEK! This is not thread safe!
    cur->second->gtk_progress_bar_pulse();
  }
  gdk_threads_leave();
}


////////////////////////////////////////////////////////////////////////
// TiledBitmapInterface

void TiledBitmap::setSource(SourcePresentation* sp)
{
  if(fileOperation==NULL)
  {
    fileOperation = LoadOperation::create(layers[0], sp, TiledBitmap::shared_from_this());
    Sequentially()->schedule(fileOperation);
  }
  else
  {
    printf("PANIC: Another operation is already in process\n");
  }
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

void TiledBitmap::drawTile(cairo_t* cr, const TileInternal::Ptr tile, const GdkRectangle viewArea)
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

void TiledBitmap::redraw(ViewInterface* vi, cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  // presentationArea.width-=200;
  // presentationArea.height-=200;
  TiledBitmapViewData::Ptr viewData = this->viewData[vi];
  
  // There's two cases: zooming in and zooming out
  if(zoom>0)
  {
    // Zooming in. This is always done using layer 0
    Layer* layer = layers[0];
    LayerOperations* layerOperations = ls[0];

    const int origWidth = presentationArea.width;
    const int origHeight = presentationArea.height;
    presentationArea.width = std::min(presentationArea.width, layer->getWidth()-presentationArea.x);
    presentationArea.height = std::min(presentationArea.height, layer->getHeight()-presentationArea.y);
    
    const int left = presentationArea.x;
    const int top = presentationArea.y;
    const int right = presentationArea.x+presentationArea.width;
    const int bottom = presentationArea.y+presentationArea.height;

    const int imin = std::min(0, left/TILESIZE);
    const int imax = (right+TILESIZE-1)/TILESIZE;
    const int jmin = std::min(0, top/TILESIZE);
    const int jmax = (bottom+TILESIZE-1)/TILESIZE;

    viewData->setNeededTiles(layer, imin, imax, jmin, jmax);

    const int pixelSize = 1<<zoom;
    
    layerOperations->initializeCairo(cr);

    if(presentationArea.width < origWidth)
    {
      GdkRectangle viewArea;
      viewArea.x = presentationArea.width*pixelSize;
      viewArea.width = (origWidth - presentationArea.width)*pixelSize;
      viewArea.y = 0;
      viewArea.height = origHeight*pixelSize;
      
      layerOperations->drawState(cr, TILE_OUT_OF_BOUNDS, viewArea);
    }
    if(presentationArea.height < origHeight)
    {
      GdkRectangle viewArea;
      viewArea.y = presentationArea.height*pixelSize;
      viewArea.height = (origHeight - presentationArea.height)*pixelSize;
      viewArea.x = 0;
      viewArea.width = presentationArea.width*pixelSize;
      
      layerOperations->drawState(cr, TILE_OUT_OF_BOUNDS, viewArea);
    }
    if(presentationArea.x<0)
    {
      GdkRectangle viewArea;
      viewArea.x=0;
      viewArea.width = -presentationArea.x*pixelSize;
      viewArea.y=0;
      viewArea.height = presentationArea.height*pixelSize;

      layerOperations->drawState(cr, TILE_OUT_OF_BOUNDS, viewArea);
    }
    if(presentationArea.y<0)
    {
      GdkRectangle viewArea;
      viewArea.y=0;
      viewArea.height = -presentationArea.y*pixelSize;
      viewArea.x = std::max(0, -presentationArea.x*pixelSize);
      viewArea.width = presentationArea.width*pixelSize;
                            
      layerOperations->drawState(cr, TILE_OUT_OF_BOUNDS, viewArea);
    }
    
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
        
        TileInternal::Ptr tile = layer->getTile(i,j);
        Tile::Ptr t = tile->getTileAsync();

        if(t)
        {
          layerOperations->draw(cr, tile->getTileAsync(), tileArea, viewArea, zoom);
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
  else
  {
    // Zooming out.

    // 1. Pick the correct layer
    unsigned int layerNr=0;
    while(zoom<=-3 && layerNr<layers.size()-1)
    {
      layerNr++;
      zoom+=3;
      presentationArea.x>>=3;
      presentationArea.y>>=3;
      presentationArea.width>>=3;
      presentationArea.height>>=3;
    }
    Layer* layer = layers[layerNr];
    LayerOperations* layerOperations = ls[std::min(ls.size()-1, (size_t)layerNr)];

    const int origWidth = presentationArea.width;
    const int origHeight = presentationArea.height;
    presentationArea.width = std::min(presentationArea.width, layer->getWidth()-presentationArea.x);
    presentationArea.height = std::min(presentationArea.height, layer->getHeight()-presentationArea.y);
    
    const int left = presentationArea.x;
    const int top = presentationArea.y;
    const int right = presentationArea.x+presentationArea.width;
    const int bottom = presentationArea.y+presentationArea.height;

    const int imin = std::max(0, left/TILESIZE);
    const int imax = (right+TILESIZE-1)/TILESIZE;
    const int jmin = std::max(0, top/TILESIZE);
    const int jmax = (bottom+TILESIZE-1)/TILESIZE;

    viewData->setNeededTiles(layer, imin, imax, jmin, jmax);

    const int pixelSize = 1<<-zoom;
    
    layerOperations->initializeCairo(cr);
    
    if(presentationArea.width < origWidth)
    {
      GdkRectangle viewArea;
      viewArea.x = presentationArea.width/pixelSize;
      viewArea.width = (origWidth - presentationArea.width)/pixelSize;
      viewArea.y = 0;
      viewArea.height = origHeight/pixelSize;
      
      layerOperations->drawState(cr, TILE_OUT_OF_BOUNDS, viewArea);
    }
    if(presentationArea.height < origHeight)
    {
      GdkRectangle viewArea;
      viewArea.y = presentationArea.height/pixelSize;
      viewArea.height = (origHeight - presentationArea.height)/pixelSize;
      viewArea.x = 0;
      viewArea.width = presentationArea.width/pixelSize;
      
      layerOperations->drawState(cr, TILE_OUT_OF_BOUNDS, viewArea);
    }
    if(presentationArea.x<0)
    {
      GdkRectangle viewArea;
      viewArea.x=0;
      viewArea.width = -presentationArea.x/pixelSize;
      viewArea.y=0;
      viewArea.height = presentationArea.height/pixelSize;

      layerOperations->drawState(cr, TILE_OUT_OF_BOUNDS, viewArea);
    }
    if(presentationArea.y<0)
    {
      GdkRectangle viewArea;
      viewArea.y=0;
      viewArea.height = -presentationArea.y/pixelSize;
      viewArea.x = std::max(0, -presentationArea.x/pixelSize);
      viewArea.width = presentationArea.width/pixelSize;
                            
      layerOperations->drawState(cr, TILE_OUT_OF_BOUNDS, viewArea);
    }
    
    
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
        
        TileInternal::Ptr tile = layer->getTile(i,j);
        Tile::Ptr t = tile->getTileAsync();

        // 3. Draw the area
        if(t)
        {
          layerOperations->draw(cr, t, tileArea, viewArea, zoom);
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
}

void TiledBitmap::open(ViewInterface* viewInterface)
{
  boost::mutex::scoped_lock lock(viewDataMutex);
  TiledBitmapViewData::Ptr vd = TiledBitmapViewData::create(viewInterface);
  viewData[viewInterface] = vd;
}

void TiledBitmap::close(ViewInterface* vi)
{
  boost::mutex::scoped_lock lock(viewDataMutex);
  viewData.erase(vi);
}

////////////////////////////////////////////////////////////////////////
// TileInitialisationObserver

void TiledBitmap::tileCreated(TileInternal::Ptr tile)
{
  UNUSED(tile);
  tileCount++;
}

void TiledBitmap::tileFinished(TileInternal::Ptr tile)
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
    gtk_progress_bar_set_fraction(((double)tileFinishedCount)/tileCount);
    if(tileFinishedCount==tileCount)
    {
      gtk_progress_bar_set_fraction(0.0);
      if(fileOperation)
      {
        fileOperation->finished();
        fileOperation.reset();
      }
      printf("INFO: Finished loading file\n");
    }
  }  
}
