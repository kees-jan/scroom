#include "tiffpresentation.hh"

#include <tiffio.h>

#include <unused.h>

TiffPresentation::TiffPresentation()
  : tif(NULL), height(0), width(0), negative(false)
{
}

TiffPresentation::~TiffPresentation()
{
  if(tif !=NULL)
  {
    TIFFClose(tif);
    tif=NULL;
  }
}

bool TiffPresentation::load(std::string fileName)
{
  tif = TIFFOpen(fileName.c_str(), "r");
    if (!tif)
  {
    // Todo: report error
    return false;
  }

  uint16 spp;
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
  if (spp != 1)
  {
    // Todo: Colour not yet supported
    return false;
  }

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

  uint16 bpp;
  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bpp);

  if(bpp!=1)
  {
    // Todo: Grayscale/colourmap not yet supported
    return false;
  }
  
  uint16 photometric;
  TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
  // In the TIFF file: black = 1, white = 0. If the values are the other way around in the TIFF file,
  // we have to swap them
  negative = false;
  if (photometric == PHOTOMETRIC_MINISBLACK || photometric == PHOTOMETRIC_PALETTE)
  {
    negative = true;
  }

  printf("This bitmap has size %d*%d\n", width, height);
  
  return true;
}
  
////////////////////////////////////////////////////////////////////////
// PresentationInterface
////////////////////////////////////////////////////////////////////////

GdkRectangle TiffPresentation::getRect()
{
  GdkRectangle rect;
  rect.x=0;
  rect.y=0;
  rect.width=width;
  rect.height=height;

  return rect;
}

void TiffPresentation::redraw(cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  // char buffer[] = "Hello world!";
  // 
  // cairo_move_to(cr, 30, 30);
  // cairo_show_text(cr, buffer);

  double pp=1.0;
  if(zoom >=0)
    pp *= 1<<zoom;
  else
    pp /= 1<<(-zoom);

  // printf("One presentation pixel amounts to %f screen pixels\n", pp);

  int xorig = (int)(-presentationArea.x*pp);
  int yorig = (int)(-presentationArea.y*pp);

  for(int i=0; i<width; i+=50)
  {
    cairo_move_to(cr, xorig+i*pp, yorig);
    cairo_line_to(cr, xorig+i*pp, yorig+height*pp);
  }
  for(int i=0; i<height; i+=50)
  {
    cairo_move_to(cr, xorig, yorig+i*pp);
    cairo_line_to(cr, xorig+width*pp, yorig+i*pp);
  }
  cairo_move_to(cr, xorig, yorig);
  cairo_line_to(cr, xorig+width*pp, yorig+height*pp);
  cairo_move_to(cr, xorig, yorig+height*pp);
  cairo_line_to(cr, xorig+width*pp, yorig);

  cairo_stroke(cr);
}
