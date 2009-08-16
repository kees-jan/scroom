#include "layeroperations.hh"

////////////////////////////////////////////////////////////////////////
// CommonOperations

void CommonOperations::initializeCairo(cairo_t* cr)
{
}

void CommonOperations::drawState(cairo_t* cr, TileState s, GdkRectangle viewArea)
{
}

////////////////////////////////////////////////////////////////////////
// Operations1bpp

int Operations1bpp::getBpp()
{
  return 1;
}

void Operations1bpp::draw(cairo_t* cr, Tile tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom)
{
}

////////////////////////////////////////////////////////////////////////
// Operations8bpp

int Operations8bpp::getBpp()
{
  return 8;
}

void Operations8bpp::draw(cairo_t* cr, Tile tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom)
{
}
