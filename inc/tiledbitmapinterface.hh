#ifndef _TILEMANAGERINTERFACE_HH
#define _TILEMANAGERINTERFACE_HH

#include <gdk/gdk.h>
#include <cairo.h>

#include <vector>

#include <viewinterface.hh>
#include <presentationinterface.hh>
#include <tile.hh>

typedef enum
  {
    TILE_UNINITIALIZED,
    TILE_UNLOADED,
    TILE_LOADED,
    TILE_OUT_OF_BOUNDS
  } TileState;

class TiledBitmapViewData : public ViewIdentifier
{
public:
  ViewInterface* viewInterface;

public:
  TiledBitmapViewData(ViewInterface* viewInterface);
};


class LayerOperations
{
public:
  virtual ~LayerOperations()
  {
  }

  virtual int getBpp()=0;
  virtual void initializeCairo(cairo_t* cr)=0;
  virtual void draw(cairo_t* cr, const Tile tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom)=0;
  virtual void drawState(cairo_t* cr, TileState s, GdkRectangle viewArea)=0;
};

typedef std::vector<LayerOperations*> LayerSpec;

class SourcePresentation
{
public:
  virtual ~SourcePresentation()
  {}

  virtual void fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile*>& tiles)=0;
};

class TiledBitmapInterface
{
public:
  virtual ~TiledBitmapInterface()
  {}

  virtual void setSource(SourcePresentation* sp)=0;
  virtual void redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom)=0;

};

TiledBitmapInterface* createTiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls);


#endif
