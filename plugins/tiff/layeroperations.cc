/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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

#include "layeroperations.hh"

#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <scroom/unused.h>

#include "tiffpresentation.hh"

using Scroom::Utils::Registration;

////////////////////////////////////////////////////////////////////////
// PixelIterator

class PixelIterator
{
private:
  byte* currentByte;
  byte currentOffset;
  const int bpp;
  const int pixelsPerByte;
  const int pixelOffset;
  const int pixelMask;

public:
  PixelIterator();
  PixelIterator(byte* base, int offset=0, int bpp=1);
  byte get();
  void set(byte value);
  PixelIterator& operator++();
  PixelIterator operator++(int);
  PixelIterator& operator+=(int x);
  byte operator*();
};

PixelIterator::PixelIterator()
  : currentByte(NULL), currentOffset(0), bpp(0), pixelsPerByte(0), pixelOffset(0), pixelMask(0)
{
}

PixelIterator::PixelIterator(byte* base, int offset, int bpp)
  : currentByte(NULL), currentOffset(0), bpp(bpp), pixelsPerByte(8/bpp), pixelOffset(bpp), pixelMask((1<<bpp)-1)
{
  div_t d = div(offset, pixelsPerByte);
  currentByte = base+d.quot;
  currentOffset = pixelsPerByte-1-d.rem;
}

inline byte PixelIterator::get()
{
  return (*currentByte>>(currentOffset*pixelOffset)) & pixelMask;
}

inline void PixelIterator::set(byte value)
{
  *currentByte =
    (*currentByte & ~(pixelMask << (currentOffset*pixelOffset))) |
    (value  << (currentOffset*pixelOffset));
}

inline byte PixelIterator::operator*()
{
  return (*currentByte>>(currentOffset*pixelOffset)) & pixelMask;
}

inline PixelIterator& PixelIterator::operator++()
{
  // Prefix operator
  if(!(currentOffset--))
  {
    currentOffset=pixelsPerByte-1;
    ++currentByte;
  }
  
  return *this;
}

inline PixelIterator PixelIterator::operator++(int)
{
  // Postfix operator
  PixelIterator result = *this;
  
  if(!(currentOffset--))
  {
    currentOffset=pixelsPerByte-1;
    ++currentByte;
  }
  
  return result;
}

PixelIterator& PixelIterator::operator+=(int x)
{
  int offset = pixelsPerByte-1-currentOffset+x;
  div_t d = div(offset, pixelsPerByte);
  currentByte += d.quot;
  currentOffset = pixelsPerByte-1-d.rem;

  return *this;
}

////////////////////////////////////////////////////////////////////////
// BitCountLut

class BitCountLut
{
private:
  byte lut[256];
public:
  BitCountLut();

  byte lookup(byte index);
};

BitCountLut bcl;

BitCountLut::BitCountLut()
{
  for(int i=0; i<256; i++)
  {
    int sum=0;
    for(int v=i;v;v>>=1)
    {
      sum += v&1;
    }
    lut[i]=sum;
  }
}

inline byte BitCountLut::lookup(byte index)
{
  return lut[index];
}

////////////////////////////////////////////////////////////////////////
// BitmapSurface

class BitmapSurface : public boost::noncopyable
{
public:
  typedef boost::shared_ptr<BitmapSurface> Ptr;

private:
  cairo_surface_t* surface;
  unsigned char* data;
  
public:
  static Ptr create(int width, int height);
  static Ptr create(int width, int height, int stride, unsigned char* data);

  ~BitmapSurface();

  cairo_surface_t* get();
private:
  BitmapSurface(int width, int heght);
  BitmapSurface(int width, int height, int stride, unsigned char* data);
};

BitmapSurface::Ptr BitmapSurface::create(int width, int height)
{
  return BitmapSurface::Ptr(new BitmapSurface(width, height));
}

BitmapSurface::Ptr BitmapSurface::create(int width, int height, int stride, unsigned char* data)
{
  return BitmapSurface::Ptr(new BitmapSurface(width, height, stride, data));
}


BitmapSurface::~BitmapSurface()
{
  cairo_surface_destroy(surface);
  if(data) free(data);
}


cairo_surface_t* BitmapSurface::get()
{
  return surface;
}

BitmapSurface::BitmapSurface(int width, int height)
{
  this->surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
  this->data = NULL;
}

BitmapSurface::BitmapSurface(int width, int height, int stride, unsigned char* data)
{
  this->surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_RGB24, width, height, stride);
  this->data = data;
}
  
////////////////////////////////////////////////////////////////////////
// CommonOperations

CommonOperations::CommonOperations(TiffPresentation* presentation)
  : presentation(presentation)
{
}

void CommonOperations::initializeCairo(cairo_t* cr)
{
  cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
}

void CommonOperations::drawState(cairo_t* cr, TileState s, GdkRectangle viewArea)
{
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
    cairo_set_source_rgb(cr, 0.75, 0.75, 1); // Blue
    break;
  }
  fillRect(cr, viewArea);
}

inline void CommonOperations::drawPixel(cairo_t* cr, int x, int y, int size, const Color& color)
{
  cairo_set_source_rgb(cr, color.red, color.green, color.blue);
  cairo_move_to(cr, x, y);
  cairo_line_to(cr, x+size, y);
  cairo_line_to(cr, x+size, y+size);
  cairo_line_to(cr, x, y+size);
  cairo_line_to(cr, x, y);
  cairo_fill(cr);
}

inline double CommonOperations::mix(double d1, double d2, byte greyscale)
{
  return (greyscale*d2 + (255-greyscale)*d1)/255;
}

void CommonOperations::drawPixel(cairo_t* cr, int x, int y, int size, const Color& c1, const Color& c2, byte greyscale)
{
  cairo_set_source_rgb(cr,
                       mix(c1.red, c2.red, greyscale),
                       mix(c1.green, c2.green, greyscale),
                       mix(c1.blue, c2.blue, greyscale));
  cairo_move_to(cr, x, y);
  cairo_line_to(cr, x+size, y);
  cairo_line_to(cr, x+size, y+size);
  cairo_line_to(cr, x, y+size);
  cairo_line_to(cr, x, y);
  cairo_fill(cr);
}


inline void CommonOperations::fillRect(cairo_t* cr, int x, int y,
                                        int width, int height)
{
  cairo_move_to(cr, x, y);
  cairo_line_to(cr, x+width, y);
  cairo_line_to(cr, x+width, y+height);
  cairo_line_to(cr, x, y+height);
  cairo_line_to(cr, x, y);
  cairo_fill(cr);
}
  
inline void CommonOperations::fillRect(cairo_t* cr, const GdkRectangle& area)
{
  fillRect(cr, area.x, area.y, area.width, area.height);
}

Scroom::Utils::Registration CommonOperations::cacheZoom(const Tile::Ptr tile, int zoom,
                                                        Scroom::Utils::Registration cache)
{
  // In: Cairo surface at zoom level 0
  // Out: Cairo surface at requested zoom level
  Scroom::Utils::Registration result;
  if(zoom>=0)
  {
    // Don't zoom in. It is a waste of space
    result = cache;
  }
  else if (!cache)
  {
    printf("PANIC: Base caching failed to return anything\n");
  }
  else
  {
    int divider = 1<<-zoom;
    BitmapSurface::Ptr source = boost::static_pointer_cast<BitmapSurface>(cache);
    BitmapSurface::Ptr target = BitmapSurface::create(tile->width/divider, tile->height/divider);
    result = target;
    
    cairo_surface_t* surface = target->get();
    cairo_t* cr = cairo_create(surface);
    initializeCairo(cr);
    cairo_scale(cr, 1.0/divider, 1.0/divider);
    cairo_set_source_surface(cr, source->get(), 0, 0);
    cairo_paint(cr);

    cairo_destroy(cr);
  }
    
  return result;
}

void CommonOperations::draw(cairo_t* cr, const Tile::Ptr tile,
                    GdkRectangle tileArea, GdkRectangle viewArea, int zoom,
                    Scroom::Utils::Registration cache)
{
  // In: Cairo surface at requested zoom level
  // Out: given surface rendered to the canvas
  UNUSED(tile);

  if(!cache)
  {
    drawState(cr, TILE_UNLOADED, viewArea);
  }
  else
  {
    BitmapSurface::Ptr source = boost::static_pointer_cast<BitmapSurface>(cache);

    if(zoom>0)
    {
      // Ask Cairo to zoom in for us
      int multiplier = 1<<zoom;
      int x = viewArea.x - multiplier * tileArea.x;
      int y = viewArea.y - multiplier * tileArea.y;

      cairo_save(cr);
      cairo_scale(cr, multiplier, multiplier);
      cairo_set_source_surface(cr, source->get(), x*multiplier, y*multiplier);
      cairo_paint(cr);
      cairo_restore(cr);
    }
    else
    {
      // Cached bitmap is to scale
      int divider = 1<<-zoom;
      int x = viewArea.x - tileArea.x / divider;
      int y = viewArea.y - tileArea.y / divider;
      cairo_set_source_surface(cr, source->get(), x, y);
      cairo_paint(cr);
    }      
  } 
}
  

////////////////////////////////////////////////////////////////////////
// Operations1bpp

Operations1bpp::Operations1bpp(TiffPresentation* presentation)
  : CommonOperations(presentation)
{
}

int Operations1bpp::getBpp()
{
  return 1;
}

Scroom::Utils::Registration Operations1bpp::cache(const Tile::Ptr tile)
{
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, tile->width);
  unsigned char* data = (unsigned char*)malloc(stride * tile->height);
  Colormap::Ptr colormap = presentation->getColormap();

  unsigned char* row = data;
  for(int j=0; j<tile->height; j++, row+=stride)
  {
    PixelIterator bit(tile->data+j*tile->width/8, 0);
    uint32_t* pixel = (uint32_t*)row;
    for(int i=0; i<tile->width; i++)
    {
      *pixel = colormap->colors[*bit].getRGB24();
      pixel++;
      ++bit;
    }
  }

  return BitmapSurface::create(tile->width, tile->height, stride, data);
}

void Operations1bpp::reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y)
{
  // Reducing by a factor 8. Source tile is 1bpp. Target tile is 8bpp
  int sourceStride = source->width/8;
  byte* sourceBase = source->data;

  int targetStride = target->width;
  byte* targetBase = target->data +
    target->height*y*targetStride/8 +
    target->width*x/8;

  for(int j=0; j<source->height/8;
      j++, targetBase+=targetStride, sourceBase+=sourceStride*8)
  {
    // Iterate vertically over target
    byte* sourcePtr = sourceBase;
    byte* targetPtr = targetBase;

    for(int i=0; i<source->width/8;
        i++, sourcePtr++, targetPtr++)
    {
      // Iterate horizontally over target

      // Goal is to compute a 8-bit grey value from a 8*8 black/white
      // image. To do so, we take each of the 8 bytes, count the
      // number of 1's in each, and add them. Finally, we divide that
      // by 64 (the maximum number of ones in that area

      byte* current = sourcePtr;
      int sum = 0;
      for(int k=0; k<8; k++, current+=sourceStride)
        sum += bcl.lookup(*current);

      *targetPtr = sum*255/64;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// Operations8bpp

Operations8bpp::Operations8bpp(TiffPresentation* presentation)
  : CommonOperations(presentation)
{
}

int Operations8bpp::getBpp()
{
  return 8;
}

void Operations8bpp::draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom, Registration cache)
{
  UNUSED(cache);
  
  cairo_set_source_rgb(cr, 1, 1, 1); // White
  fillRect(cr, viewArea);
  Colormap::Ptr colormap = presentation->getColormap();
  const Color& c1 = colormap->colors[0];
  const Color& c2 = colormap->colors[1];
  
  if(zoom>=0)
  {
    // Iterate over pixels in the tileArea, drawing each as we go ahead
    int pixelSize = 1<<zoom;

    for(int j=0; j<tileArea.height; j++)
    {
      byte* cur = tile->data+(tileArea.y+j)*tile->width + tileArea.x;
    
      for(int i=0; i<tileArea.width; i++, cur++)
      {
        drawPixel(cr, viewArea.x+i*pixelSize, viewArea.y+j*pixelSize, pixelSize, c1, c2, *cur);
      }
    }
  }
  else
  {
    // zoom < 0
    // Iterate over pixels in the viewArea, determining which color
    // each should get
    int pixelCount = 1<<-zoom;

    for(int j=0; j<viewArea.height; j++)
    {
      byte* cur = tile->data +
        (tileArea.y+j*pixelCount)*tile->width +
        tileArea.x;
    
      for(int i=0; i<viewArea.width; i++)
      {
        int count=0;
        byte* bl = cur;
        for(int l=0; l<pixelCount; l++)
        {
          byte* bk = bl;
          for(int k=0; k<pixelCount; k++)
          {
            count += *bk;
            ++bk;
          }

          bl+= tile->width;
        }
        
        drawPixel(cr, viewArea.x+i, viewArea.y+j, 1, c1, c2, count/pixelCount/pixelCount);
        cur+=pixelCount;
      }
    }
  }
}

void Operations8bpp::reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y)
{
  // Reducing by a factor 8. Source tile is 8bpp. Target tile is 8bpp
  int sourceStride = source->width;
  byte* sourceBase = source->data;

  int targetStride = target->width;
  byte* targetBase = target->data +
    target->height*y*targetStride/8 +
    target->width*x/8;

  for(int j=0; j<source->height/8;
      j++, targetBase+=targetStride, sourceBase+=sourceStride*8)
  {
    // Iterate vertically over target
    byte* sourcePtr = sourceBase;
    byte* targetPtr = targetBase;

    for(int i=0; i<source->width/8;
        i++, sourcePtr+=8, targetPtr++)
    {
      // Iterate horizontally over target

      // Goal is to compute a 8-bit grey value from a 8*8 grey image.
      byte* base = sourcePtr;
      int sum = 0;
      for(int k=0; k<8; k++, base+=sourceStride)
      {
        byte* current=base;
        for(int l=0; l<8; l++, current++)
          sum += *current;
      }

      *targetPtr = sum/64;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// Operations

Operations::Operations(TiffPresentation* presentation, int bpp)
  : CommonOperations(presentation), 
    bpp(bpp), pixelsPerByte(8/bpp), pixelOffset(bpp), pixelMask((1<<bpp)-1)
{
}

int Operations::getBpp()
{
  return bpp;
}

void Operations::draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom, Registration cache)
{
  UNUSED(cache);
  cairo_set_source_rgb(cr, 1, 1, 1); // White
  fillRect(cr, viewArea);
  Colormap::Ptr colormap = presentation->getColormap();
  
  if(zoom>=0)
  {
    // Iterate over pixels in the tileArea, drawing each as we go ahead
    int pixelSize = 1<<zoom;

    for(int j=0; j<tileArea.height; j++)
    {
      PixelIterator pixel(tile->data+(tileArea.y+j)*tile->width/pixelsPerByte, tileArea.x, bpp);
      for(int i=0; i<tileArea.width; i++)
      {
        drawPixel(cr, viewArea.x+i*pixelSize, viewArea.y+j*pixelSize, pixelSize, colormap->colors[*pixel]);
        ++pixel;
      }
    }
  }
  else
  {
    // zoom < 0
    // Iterate over pixels in the viewArea, determining which color
    // each should get
    int pixelCount = 1<<-zoom;

    for(int j=0; j<viewArea.height; j++)
    {
      PixelIterator pixel(tile->data+(tileArea.y+j*pixelCount)*tile->width/pixelsPerByte,
                          tileArea.x, bpp);
      
      for(int i=0; i<viewArea.width; i++)
      {
        // Compute the average of all colors in a
        // pixelCount*pixelCount area
        Color c;
        //   PixelIterator bl(pixel);
        //   for(int l=0; l<pixelCount; l++)
        //   {
        //     PixelIterator bk(bl);
        //     for(int k=0; k<pixelCount; k++)
        //     {
        //       c += colormap->colors[*bk];
        //       ++bk;
        //     }
        //     bl+= tile->width;
        //   }
        // 
        //   c /= pixelCount*pixelCount;

        // Goal is to determine which values occurs most often in a
        // pixelCount*pixelCount rectangle, and pick the top two.
        unsigned lookup[pixelMask+1];
        memset(lookup, 0, sizeof(lookup));

        PixelIterator bl(pixel);
        for(int l=0; l<pixelCount; l++)
        {
          PixelIterator bk(bl);
          for(int k=0; k<pixelCount; k++)
          {
            ++(lookup[*bk]);
            ++bk;
          }
          bl+= tile->width;
        }

        byte first=0;
        byte second=1;
        if(lookup[1]>lookup[0])
        {
          first=1;
          second=0;
        }
        for(int b=2; b<pixelMask+1; b++)
        {
          if(lookup[b]>lookup[first])
          {
            second=first;
            first=b;
          }
          else if(lookup[b]>lookup[second])
            second=b;
        }
        if(lookup[second]==0)
          second = first;

        c += colormap->colors[first];
        c += colormap->colors[second];
        c /= 2;
        
        drawPixel(cr, viewArea.x+i, viewArea.y+j, 1, c);
        pixel+=pixelCount;
      }
    }
  }
}

void Operations::reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y)
{
  // Reducing by a factor 8. Target is 2*bpp and expects two indices into the colormap
  int sourceStride = source->width/pixelsPerByte;
  byte* sourceBase = source->data;

  const int targetMultiplier = 2; // target is 2*bpp
  int targetStride = targetMultiplier * target->width / pixelsPerByte;
  byte* targetBase = target->data +
    target->height*targetStride*y/8 +
    targetMultiplier*target->width*x/8/pixelsPerByte;

  for(int j=0; j<source->height/8;
      j++, targetBase+=targetStride, sourceBase+=sourceStride*8)
  {
    // Iterate vertically over target
    byte* sourcePtr = sourceBase;
    PixelIterator targetPtr(targetBase, 0, targetMultiplier * bpp);

    for(int i=0; i<source->width/8;
        i++, sourcePtr+=8/pixelsPerByte, ++targetPtr)
    {
      // Iterate horizontally over target

      // Goal is to determine which values occurs most often in a 8*8
      // rectangle, and pick the top two.
      byte* base = sourcePtr;
      byte lookup[pixelMask+1];
      memset(lookup, 0, sizeof(lookup));

      for(int k=0; k<8; k++, base+=sourceStride)
      {
        PixelIterator current(base, 0, bpp);
        for(int l=0; l<8; l++, ++current)
          ++(lookup[*current]);
      }
      byte first=0;
      byte second=1;
      if(lookup[1]>lookup[0])
      {
        first=1;
        second=0;
      }
      for(byte c=2; c<pixelMask+1; c++)
      {
        if(lookup[c]>lookup[first])
        {
          second=first;
          first=c;
        }
        else if(lookup[c]>lookup[second])
          second=c;
      }
      if(lookup[second]==0)
        second = first;

      targetPtr.set(first<<pixelOffset | second);
    }
  }
}

////////////////////////////////////////////////////////////////////////
// OperationsColormapped

OperationsColormapped::OperationsColormapped(TiffPresentation* presentation, int bpp)
  : Operations(presentation, bpp)
{
}

int OperationsColormapped::getBpp()
{
  return 2*bpp;
}

void OperationsColormapped::draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom, Registration cache)
{
  UNUSED(cache);
  
  cairo_set_source_rgb(cr, 1, 1, 1); // White
  fillRect(cr, viewArea);
  Colormap::Ptr colormap = presentation->getColormap();
  const int multiplier = 2; // data is 2*bpp, containing 2 colors

  g_assert_cmpint(zoom, <=, 0);
  
  if(zoom<=0)
  {
    // zoom <= 0
    // Iterate over pixels in the viewArea, determining which color
    // each should get
    int pixelCount = 1<<-zoom;

    for(int j=0; j<viewArea.height; j++)
    {
      PixelIterator pixel(tile->data+(tileArea.y+j*pixelCount)*multiplier*tile->width/pixelsPerByte,
                          tileArea.x, multiplier*bpp);
      
      for(int i=0; i<viewArea.width; i++)
      {
        // Compute the average of all colors in a
        // pixelCount*pixelCount area
        Color c;
        PixelIterator bl(pixel);
        for(int l=0; l<pixelCount; l++)
        {
          PixelIterator bk(bl);
          for(int k=0; k<pixelCount; k++)
          {
            c += colormap->colors[*bk & pixelMask];
            c += colormap->colors[*bk >> pixelOffset];
            ++bk;
          }
          bl+= tile->width;
        }

        c /= 2*pixelCount*pixelCount;
        
        drawPixel(cr, viewArea.x+i, viewArea.y+j, 1, c);
        pixel+=pixelCount;
      }
    }
  }
}

void OperationsColormapped::reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y)
{
  // Reducing by a factor 8. Source and target both 2*bpp, containing 2 colors
  const int multiplier = 2; // data is 2*bpp, containing 2 colors
  int sourceStride = multiplier*source->width/pixelsPerByte;
  byte* sourceBase = source->data;

  int targetStride = multiplier*target->width/pixelsPerByte;
  byte* targetBase = target->data +
    target->height*y*targetStride/8 +
    multiplier*target->width*x/8/pixelsPerByte;

  for(int j=0; j<source->height/8;
      j++, targetBase+=targetStride, sourceBase+=sourceStride*8)
  {
    // Iterate vertically over target
    byte* sourcePtr = sourceBase;
    PixelIterator targetPtr(targetBase, 0, multiplier*bpp);

    for(int i=0; i<source->width/8;
        i++, sourcePtr+=8*multiplier/pixelsPerByte, ++targetPtr)
    {
      // Iterate horizontally over target

      // Goal is to determine which value occurs most often in a 8*8
      // rectangle, and pick that value.
      byte* base = sourcePtr;
      byte lookup[pixelMask+1];
      memset(lookup, 0, sizeof(lookup));

      for(int k=0; k<8; k++, base+=sourceStride)
      {
        PixelIterator current(base, 0, multiplier*bpp);
        for(int l=0; l<8; l++, ++current)
        {
          ++lookup[*current & pixelMask];
          ++lookup[*current >> pixelOffset];
        }
      }
      byte first=0;
      byte second=1;
      if(lookup[1]>lookup[0])
      {
        first=1;
        second=0;
      }
      for(byte c=2; c<pixelMask+1; c++)
      {
        if(lookup[c]>lookup[first])
        {
          second=first;
          first=c;
        }
        else if(lookup[c]>lookup[second])
          second=c;
      }
      if(lookup[second]==0)
        second = first;

      targetPtr.set(first<<pixelOffset | second);
    }
  }
}

