#ifndef _TILED_BITMAP_HH
#define _TILED_BITMAP_HH

#include <tiledbitmapinterface.hh>

class TiledBitmap : public TiledBitmapInterface
{
private:
  int bitmapWidth;
  int bitmapHeight;
  LayerSpec ls;
  
public:
  TiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls);
  virtual ~TiledBitmap();


  ////////////////////////////////////////////////////////////////////////
  // TiledBitmapInterface
  
  virtual void setSource(SourcePresentation* sp);
  virtual void redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom);
};

#endif
