#include "tiled-bitmap.hh"

#include <stdio.h>

#include <unused.h>

TiledBitmapInterface* createTiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls)
{
  return new TiledBitmap(bitmapWidth, bitmapHeight, ls);
}

////////////////////////////////////////////////////////////////////////
// TiledBitmapViewData

TiledBitmapViewData::TiledBitmapViewData(ViewInterface* viewInterface)
  : viewInterface(viewInterface)
{
}

////////////////////////////////////////////////////////////////////////
// TiledBitmap

TiledBitmap::TiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls)
  :bitmapWidth(bitmapWidth), bitmapHeight(bitmapHeight), ls(ls)
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

    Layer* layer = new Layer(i, width, height, bpp);
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
  while(!coordinators.empty())
  {
    delete coordinators.back();
    coordinators.pop_back();
  }
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

  std::vector<LayerCoordinator*> coordinators;
  
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
        LayerCoordinator* lc = new LayerCoordinator(til[z], prevLo);
        coordinators.push_back(lc);
        this->coordinators.push_back(lc);
      }
    }

    LayerCoordinator* lc=NULL;
    
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


////////////////////////////////////////////////////////////////////////
// TiledBitmapInterface

void TiledBitmap::setSource(SourcePresentation* sp)
{
  layers[0]->fetchData(sp);
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

void TiledBitmap::drawTile(cairo_t* cr, const TileInternal* tile, const GdkRectangle viewArea)
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

void TiledBitmap::redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  // presentationArea.width-=200;
  // presentationArea.height-=200;
  
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
        
        TileInternal* tile = layer->getTile(i,j);

        if(tile->state == TILE_LOADED)
        {
          layerOperations->draw(cr, tile->getTile(), tileArea, viewArea, zoom);
        }
        else
        {
          layerOperations->drawState(cr, tile->state, viewArea);
        }
        drawTile(cr, tile, viewArea);
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
        
        TileInternal* tile = layer->getTile(i,j);

        // 3. Draw the area
        if(tile->state == TILE_LOADED)
        {
          layerOperations->draw(cr, tile->getTile(), tileArea, viewArea, zoom);
        }
        else
        {
          layerOperations->drawState(cr, tile->state, viewArea);
        }
        drawTile(cr, tile, viewArea);
      }
    }
  }
}
