#include "tiffpresentation.hh"

#include <tiffio.h>

#include <unused.h>

#include "layeroperations.hh"

TiffPresentation::TiffPresentation()
  : tif(NULL), height(0), width(0), negative(false), tbi(NULL)
{
}

TiffPresentation::~TiffPresentation()
{
  if(tif !=NULL)
  {
    TIFFClose(tif);
    tif=NULL;
  }

  while(!ls.empty())
  {
    delete ls.back();
    ls.pop_back();
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
  ls.clear();
  ls.push_back(new Operations1bpp());
  ls.push_back(new Operations8bpp());
  tbi = createTiledBitmap(width, height, ls);
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
  if(tbi)
    tbi->redraw(cr, presentationArea, zoom);
}
