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
  do
  {
    if(i<ls.size())
      bpp = ls[i]->getBpp();
    
    layers.push_back(new Layer(i, width, height, bpp));
    width = (width+7)/8; // Round up
    height = (height+7)/8;
    i++;
  } while (std::max(width, height) > TILESIZE);
}

TiledBitmap::~TiledBitmap()
{
  while(!layers.empty())
  {
    delete layers.back();
    layers.pop_back();
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
    tileSize = presentationBegin + presentationSize - tileOffset;
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
    tileSize = presentationBegin + presentationSize - tileOffset;
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
    int left = presentationArea.x;
    int top = presentationArea.y;
    int right = presentationArea.x+presentationArea.width;
    int bottom = presentationArea.y+presentationArea.height;

    int imin = left/TILESIZE;
    int imax = (right+TILESIZE-1)/TILESIZE;
    int jmin = top/TILESIZE;
    int jmax = (bottom+TILESIZE-1)/TILESIZE;

    Layer* layer = layers[0];
    LayerOperations* layerOperations = ls[0];

    layerOperations->initializeCairo(cr);
    
    for(int i=imin; i<imax; i++)
    {
      for(int j=jmin; j<jmax; j++)
      {
        int pixelSize = 1<<zoom;
        
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
        else if (tile->state != TILE_OUT_OF_BOUNDS)
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
    int layerNr=0;
    while(zoom<=-3)
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

    int left = presentationArea.x;
    int top = presentationArea.y;
    int right = presentationArea.x+presentationArea.width;
    int bottom = presentationArea.y+presentationArea.height;

    int imin = left/TILESIZE;
    int imax = (right+TILESIZE-1)/TILESIZE;
    int jmin = top/TILESIZE;
    int jmax = (bottom+TILESIZE-1)/TILESIZE;

    layerOperations->initializeCairo(cr);
    
    for(int i=imin; i<imax; i++)
    {
      for(int j=jmin; j<jmax; j++)
      {
        int pixelSize = 1<<-zoom;
        
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
