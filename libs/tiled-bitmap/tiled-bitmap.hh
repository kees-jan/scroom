#ifndef _TILED_BITMAP_HH
#define _TILED_BITMAP_HH

#include <tiledbitmapinterface.hh>

#include <list>

#include "layer.hh"
#include "layercoordinator.hh"

class TiledBitmap : public TiledBitmapInterface
{
private:
  int bitmapWidth;
  int bitmapHeight;
  LayerSpec ls;
  std::vector<Layer*> layers;
  std::list<LayerCoordinator*> coordinators;
  
public:
  TiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls);
  virtual ~TiledBitmap();

private:
  void drawTile(cairo_t* cr, const TileInternal* tile, const GdkRectangle viewArea);
  void connect(Layer* layer, Layer* prevLayer, LayerOperations* prevLo);

  ////////////////////////////////////////////////////////////////////////
  // TiledBitmapInterface

public:
  virtual void setSource(SourcePresentation* sp);
  virtual void redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom);

};

#endif
