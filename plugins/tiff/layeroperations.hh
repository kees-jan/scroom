#ifndef _TIFF_LAYEROPERATIONS_HH
#define _TIFF_LAYEROPERATIONS_HH

#include <tiledbitmapinterface.hh>

class CommonOperations : public LayerOperations
{
public:
  virtual ~CommonOperations()
  {}

  void drawPixel(cairo_t* cr, int x, int y, int size, byte greyShade=255);
  void drawPixel(cairo_t* cr, int x, int y, int size, double greyShade);
  void fillRect(cairo_t* cr, int x, int y, int width, int height);
  void fillRect(cairo_t* cr, const GdkRectangle& area);
  
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
  virtual void reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y);
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
  virtual void reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y);
};


#endif
