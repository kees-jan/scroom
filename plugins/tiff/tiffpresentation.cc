#include "tiffpresentation.hh"

#include <tiffio.h>
#include <string.h>

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
  tbi->setSource(this);
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

ViewIdentifier* TiffPresentation::open(ViewInterface* viewInterface)
{
  TiledBitmapViewData* vd = new TiledBitmapViewData(viewInterface);
  viewData[vd] = viewInterface;
  return vd;
}

void TiffPresentation::close(ViewIdentifier* vid)
{
  TiledBitmapViewData* vd = dynamic_cast<TiledBitmapViewData*>(vid);
  viewData.erase(vd);
  delete vd;
}

void TiffPresentation::redraw(ViewIdentifier* vid, cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  if(tbi)
    tbi->redraw(cr, presentationArea, zoom);
}

////////////////////////////////////////////////////////////////////////
// SourcePresentation
////////////////////////////////////////////////////////////////////////

void TiffPresentation::fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles)
{
  printf("Filling lines %d to %d, tile %d to %d (tileWidth = %d)\n",
         startLine, startLine+lineCount,
         firstTile, (int)(firstTile+tiles.size()),
         tileWidth);

  int dataLength = (width+7)/8;
  byte row[dataLength];

  int tileCount = tiles.size();
  byte* dataPtr[tileCount];
  for(int tile=0; tile<tileCount; tile++)
  {
    dataPtr[tile] = tiles[tile]->data;
  }

  for(int i=0; i<lineCount; i++)
  {
    TIFFReadScanline(tif, (tdata_t)row, startLine+i);
    if(negative)
    {
      for(int j=0; j<dataLength; j++)
        row[j] = ~row[j];
    }

    for(int tile=0; tile<tileCount-1; tile++)
    {
      memcpy(dataPtr[tile], row+(firstTile+tile)*tileWidth/8, tileWidth/8);
      dataPtr[tile]+=tileWidth/8;
    }
    memcpy(dataPtr[tileCount-1],
           row+(firstTile+tileCount-1)*tileWidth/8,
           dataLength - (firstTile+tileCount-1)*tileWidth/8);
  }
}


