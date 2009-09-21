#ifndef _TIFF_LAYEROPERATIONS_HH
#define _TIFF_LAYEROPERATIONS_HH

#include <tiledbitmapinterface.hh>

class CommonOperations : public LayerOperations
{
public:
  virtual ~CommonOperations()
  {}

  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual void initializeCairo(cairo_t* cr);
  virtual void drawState(cairo_t* cr, TileState s, GdkRectangle viewArea);
};

class Operations1bpp : public CommonOperations
{
public:
  virtual ~Operations1bpp()
  {}
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual int getBpp();
  virtual void draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom);
};

class Operations8bpp : public CommonOperations
{
public:
  virtual ~Operations8bpp()
  {}
  
  ////////////////////////////////////////////////////////////////////////
  // LayerOperations

  virtual int getBpp();
  virtual void draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom);
};


#endif
