#include "layeroperations.hh"

#include <stdio.h>

////////////////////////////////////////////////////////////////////////
// CommonOperations

void CommonOperations::initializeCairo(cairo_t* cr)
{
  printf("InitializeCairo\n");
  cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
}

void CommonOperations::drawState(cairo_t* cr, TileState s, GdkRectangle viewArea)
{
  printf("DrawState\n");
  switch(s)
  {
  case TILE_UNINITIALIZED:
    cairo_set_source_rgb(cr, 1, 1, 0); // Yellow
    break;
  case TILE_UNLOADED:
    cairo_set_source_rgb(cr, 0, 1, 0); // Green
    break;
  case TILE_LOADED:
    cairo_set_source_rgb(cr, 1, 0, 0); // Red
    break;
  case TILE_OUT_OF_BOUNDS:
  default:
    cairo_set_source_rgb(cr, 0, 0, 1); // Blue
    break;
  }
  cairo_move_to(cr, viewArea.x, viewArea.y);
  cairo_line_to(cr, viewArea.x+viewArea.width, viewArea.y);
  cairo_line_to(cr, viewArea.x+viewArea.width, viewArea.y+viewArea.height);
  cairo_line_to(cr, viewArea.x, viewArea.y+viewArea.height);
  cairo_line_to(cr, viewArea.x, viewArea.y);
  cairo_fill(cr);
}

////////////////////////////////////////////////////////////////////////
// Operations1bpp

int Operations1bpp::getBpp()
{
  return 1;
}

void Operations1bpp::draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom)
{
  printf("Draw1bpp\n");
}

////////////////////////////////////////////////////////////////////////
// Operations8bpp

int Operations8bpp::getBpp()
{
  return 8;
}

void Operations8bpp::draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom)
{
  printf("Draw8bpp\n");
}
