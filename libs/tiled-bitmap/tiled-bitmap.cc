#include "tiled-bitmap.hh"

TiledBitmapInterface* createTiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls)
{
  return new TiledBitmap(bitmapWidth, bitmapHeight, ls);
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
}

void TiledBitmap::redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  double pp=1.0;
  if(zoom >=0)
    pp *= 1<<zoom;
  else
    pp /= 1<<(-zoom);

  int xorig = (int)(-presentationArea.x*pp);
  int yorig = (int)(-presentationArea.y*pp);

  for(int i=0; i<bitmapWidth; i+=50)
  {
    cairo_move_to(cr, xorig+i*pp, yorig);
    cairo_line_to(cr, xorig+i*pp, yorig+bitmapHeight*pp);
  }
  for(int i=0; i<bitmapHeight; i+=50)
  {
    cairo_move_to(cr, xorig, yorig+i*pp);
    cairo_line_to(cr, xorig+bitmapWidth*pp, yorig+i*pp);
  }
  cairo_move_to(cr, xorig, yorig);
  cairo_line_to(cr, xorig+bitmapWidth*pp, yorig+bitmapHeight*pp);
  cairo_move_to(cr, xorig, yorig+bitmapHeight*pp);
  cairo_line_to(cr, xorig+bitmapWidth*pp, yorig);

  cairo_stroke(cr);

}
