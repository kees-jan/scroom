/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "tiffpresentation.hh"

#include <tiffio.h>
#include <string.h>

#include <scroom/unused.h>

#include "layeroperations.hh"

TiffPresentation::TiffPresentation()
  : fileName(), tif(NULL), height(0), width(0), tbi(NULL), bpp(0)
{
  colormap = Colormap::createDefault(256);
}

TiffPresentation::~TiffPresentation()
{
  if(tif != NULL)
  {
    TIFFClose(tif);
    tif=NULL;
  }

  if(tbi != NULL)
  {
    delete tbi;
    tbi=NULL;
  }

  while(!ls.empty())
  {
    delete ls.back();
    ls.pop_back();
  }
}

bool TiffPresentation::load(std::string fileName, FileOperationObserver* observer)
{
  this->fileName = fileName;
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
  this->bpp = bpp;

  uint16 *r, *g, *b;
  int result = TIFFGetField(tif, TIFFTAG_COLORMAP, &r, &g, &b);
  if(result==1)
  {
    originalColormap = Colormap::create();
    originalColormap->name = "Original";
    int count = 1<<bpp;
    originalColormap->colors.resize(count);
    
    for(int i=0; i<count; i++)
    {
      originalColormap->colors[i] = Color(1.0*r[i]/0xFFFF, 1.0*g[i]/0xFFFF, 1.0*b[i]/0xFFFF);
    }
  }
  else
  {
    colormap = Colormap::createDefault(1<<bpp);
  }
  
  uint16 photometric;
  TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
  switch(photometric)
  {
  case PHOTOMETRIC_MINISBLACK:
    if(!originalColormap)
      originalColormap = Colormap::createDefault(1<<bpp);
    else
      printf("WEIRD: Tiff contains a colormap, but photometric isn't palette\n");
    break;
      
  case PHOTOMETRIC_MINISWHITE:
    if(!originalColormap)
      originalColormap = Colormap::createDefaultInverted(1<<bpp);
    else
      printf("WEIRD: Tiff contains a colormap, but photometric isn't palette\n");
    break;
      
  case PHOTOMETRIC_PALETTE:
    if(!originalColormap)
    {
      printf("WEIRD: Photometric is palette, but tiff doesn't contain a colormap\n");
      originalColormap = Colormap::createDefault(1<<bpp);
    }
    break;
      
  default:
    printf("PANIC: Unrecognized value for photometric\n");
    break;
  }

  colormap = originalColormap;
    
  printf("This bitmap has size %d*%d\n", width, height);
  ls.clear();

  if(bpp==1)
  {
    ls.push_back(new Operations1bpp());
    ls.push_back(new Operations8bpp());
  }
  else if(bpp==8 && photometric != PHOTOMETRIC_PALETTE)
  {
    ls.push_back(new Operations8bpp());
  }
  else if(bpp==2 || bpp==4 || (bpp==8 && photometric == PHOTOMETRIC_PALETTE))
  {
    ls.push_back(new Operations(this, bpp));
    properties[COLORMAPPABLE_PROPERTY_NAME]="";
  }
  else
  {
    return false;
  }

  tbi = createTiledBitmap(width, height, ls, observer);
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

void TiffPresentation::open(ViewInterface* viewInterface)
{
  views.push_back(viewInterface);
  std::list<Viewable*> observers = getObservers();
  for(std::list<Viewable*>::iterator cur=observers.begin();
      cur!=observers.end(); ++cur)
  {
    (*cur)->open(viewInterface);
  }
  
  if(tbi)
    tbi->open(viewInterface);
  else
  {
    printf("ERROR: TiffPresentation::open(): No TiledBitmapInterface available!\n");
  }
}

void TiffPresentation::close(ViewInterface* vi)
{
  views.remove(vi);
  std::list<Viewable*> observers = getObservers();
  for(std::list<Viewable*>::iterator cur=observers.begin();
      cur!=observers.end(); ++cur)
  {
    (*cur)->close(vi);
  }
  
  if(tbi)
    tbi->close(vi);
  else
  {
    printf("ERROR: TiffPresentation::close(): No TiledBitmapInterface available!\n");
  }
}

void TiffPresentation::redraw(ViewInterface* vi, cairo_t* cr, GdkRectangle presentationArea, int zoom)
{
  if(tbi)
    tbi->redraw(vi, cr, presentationArea, zoom);
}

bool TiffPresentation::getProperty(const std::string& name, std::string& value)
{
  std::map<std::string, std::string>::iterator p = properties.find(name);
  bool found = false;
  if(p == properties.end())
  {
    found = false;
    value = "";
  }
  else
  {
    found = true;
    value = p->second;
  }
  
  return found;
}

bool TiffPresentation::isPropertyDefined(const std::string& name)
{
  return properties.end() != properties.find(name);
}

std::string TiffPresentation::getTitle()
{
  return fileName;
}


////////////////////////////////////////////////////////////////////////
// SourcePresentation
////////////////////////////////////////////////////////////////////////

void TiffPresentation::fillTiles(int startLine, int lineCount, int tileWidth, int firstTile, std::vector<Tile::Ptr>& tiles)
{
  // printf("Filling lines %d to %d, tile %d to %d (tileWidth = %d)\n",
  //        startLine, startLine+lineCount,
  //        firstTile, (int)(firstTile+tiles.size()),
  //        tileWidth);

  int pixelsPerByte = 8/bpp;
  int dataLength = (width+pixelsPerByte-1)/pixelsPerByte;
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

    for(int tile=0; tile<tileCount-1; tile++)
    {
      memcpy(dataPtr[tile], row+(firstTile+tile)*tileWidth/pixelsPerByte, tileWidth/pixelsPerByte);
      dataPtr[tile]+=tileWidth/pixelsPerByte;
    }
    memcpy(dataPtr[tileCount-1],
           row+(firstTile+tileCount-1)*tileWidth/pixelsPerByte,
           dataLength - (firstTile+tileCount-1)*tileWidth/pixelsPerByte);
    dataPtr[tileCount-1]+=tileWidth/pixelsPerByte;
  }
}

////////////////////////////////////////////////////////////////////////
// Colormappable
////////////////////////////////////////////////////////////////////////

void TiffPresentation::registerObserver(Viewable* observer)
{
  Colormappable::registerObserver(observer);

  for(std::list<ViewInterface*>::iterator cur=views.begin();
      cur!=views.end(); ++cur)
  {
    observer->open(*cur);
  }
}

void TiffPresentation::setColormap(Colormap::Ptr colormap)
{
  for(std::list<ViewInterface*>::iterator cur=views.begin();
      cur!=views.end(); ++cur)
  {
    (*cur)->invalidate();
  }
  this->colormap = colormap;
}

Colormap::Ptr TiffPresentation::getColormap()
{
  return colormap;
}

Colormap::Ptr TiffPresentation::getOriginalColormap()
{
  return originalColormap;
}

int TiffPresentation::getNumberOfColors()
{
  return 1<<bpp;
}

////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////

