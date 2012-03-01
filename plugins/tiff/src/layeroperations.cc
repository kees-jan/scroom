/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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
#include <stdint.h>
#include <string.h>

#include <glib.h>

#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <scroom/unused.h>

#include "tiffpresentation.hh"

using Scroom::Utils::Stuff;

////////////////////////////////////////////////////////////////////////
// PixelIterator

template <typename Base>
class PixelIterator
{
private:
  Base* currentBase;
  int currentOffset;
  const int bpp;
  const int pixelsPerBase;
  static const int bitsPerBase;
  const int pixelOffset;
  const Base pixelMask;

private:
  static Base mask(int bpp);
  
public:
  PixelIterator();
  PixelIterator(Base* base, int offset=0, int bpp=1);
  Base get();
  void set(Base value);
  PixelIterator& operator++();
  PixelIterator operator++(int);
  PixelIterator& operator+=(int x);
  Base operator*();
};

template <typename Base>
const int PixelIterator<Base>::bitsPerBase = 8*sizeof(Base)/sizeof(byte);

template <typename Base>
Base PixelIterator<Base>::mask(int bpp)
{
  return (((Base(1) << (bpp - 1)) - 1) << 1) | 1;
}

template <typename Base>
PixelIterator<Base>::PixelIterator()
  : currentBase(NULL), currentOffset(0), bpp(0), pixelsPerBase(0), pixelOffset(0), pixelMask(0)
{
}

template <typename Base>
PixelIterator<Base>::PixelIterator(Base* base, int offset, int bpp)
  : currentBase(NULL), currentOffset(0), bpp(bpp), pixelsPerBase(bitsPerBase/bpp), pixelOffset(bpp), pixelMask(mask(bpp))
{
  div_t d = div(offset, pixelsPerBase);
  currentBase = base+d.quot;
  currentOffset = pixelsPerBase-1-d.rem;
}

template <typename Base>
inline Base PixelIterator<Base>::get()
{
  return (*currentBase>>(currentOffset*pixelOffset)) & pixelMask;
}

template <typename Base>
inline void PixelIterator<Base>::set(Base value)
{
  *currentBase =
    (*currentBase & ~(pixelMask << (currentOffset*pixelOffset))) |
    (value  << (currentOffset*pixelOffset));
}

template <typename Base>
inline Base PixelIterator<Base>::operator*()
{
  return (*currentBase>>(currentOffset*pixelOffset)) & pixelMask;
}

template <typename Base>
inline PixelIterator<Base>& PixelIterator<Base>::operator++()
{
  // Prefix operator
  if(!(currentOffset--))
  {
    currentOffset=pixelsPerBase-1;
    ++currentBase;
  }
  
  return *this;
}

template <typename Base>
inline PixelIterator<Base> PixelIterator<Base>::operator++(int)
{
  // Postfix operator
  PixelIterator<Base> result = *this;
  
  if(!(currentOffset--))
  {
    currentOffset=pixelsPerBase-1;
    ++currentBase;
  }
  
  return result;
}

template <typename Base>
PixelIterator<Base>& PixelIterator<Base>::operator+=(int x)
{
  int offset = pixelsPerBase-1-currentOffset+x;
  div_t d = div(offset, pixelsPerBase);
  currentBase += d.quot;
  currentOffset = pixelsPerBase-1-d.rem;

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
  setClip(cr, viewArea);
  cairo_paint(cr);
}

inline void CommonOperations::setClip(cairo_t* cr, int x, int y,
                                      int width, int height)
{
  cairo_move_to(cr, x, y);
  cairo_line_to(cr, x+width, y);
  cairo_line_to(cr, x+width, y+height);
  cairo_line_to(cr, x, y+height);
  cairo_line_to(cr, x, y);
  cairo_clip(cr);
}
  
inline void CommonOperations::setClip(cairo_t* cr, const GdkRectangle& area)
{
  setClip(cr, area.x, area.y, area.width, area.height);
}

void CommonOperations::drawPixelValue(cairo_t* cr, int x, int y, int size, int value)
{
  std::ostringstream s;
  s << value;
  std::string v = s.str();
  const char* cstr = v.c_str();

  cairo_move_to(cr, x, y);
  cairo_line_to(cr, x+size, y);
  cairo_line_to(cr, x+size, y+size);
  cairo_line_to(cr, x, y+size);
  cairo_line_to(cr, x, y);
  cairo_clip(cr);
  // cairo_stroke(cr);

  cairo_text_extents_t extents;
  cairo_text_extents(cr, cstr, &extents);

  double xx = x+size/2-(extents.width/2 + extents.x_bearing);
  double yy = y+size/2-(extents.height/2 + extents.y_bearing);

  cairo_move_to(cr, xx, yy);
  cairo_show_text(cr, cstr);
}

Scroom::Utils::Stuff CommonOperations::cacheZoom(const ConstTile::Ptr tile, int zoom,
                                                        Scroom::Utils::Stuff cache)
{
  // In: Cairo surface at zoom level 0
  // Out: Cairo surface at requested zoom level
  Scroom::Utils::Stuff result;
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

void CommonOperations::draw(cairo_t* cr, const ConstTile::Ptr tile,
                    GdkRectangle tileArea, GdkRectangle viewArea, int zoom,
                    Scroom::Utils::Stuff cache)
{
  // In: Cairo surface at requested zoom level
  // Out: given surface rendered to the canvas
  UNUSED(tile);

  setClip(cr, viewArea);
  
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
      int x = viewArea.x / multiplier - tileArea.x;
      int y = viewArea.y / multiplier - tileArea.y;

      cairo_scale(cr, multiplier, multiplier);
      cairo_set_source_surface(cr, source->get(), x, y);
      cairo_pattern_set_filter (cairo_get_source (cr), CAIRO_FILTER_NEAREST);
      cairo_paint(cr);
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

LayerOperations::Ptr Operations1bpp::create(TiffPresentation* presentation)
{
  return Ptr(new Operations1bpp(presentation));
}

Operations1bpp::Operations1bpp(TiffPresentation* presentation)
  : CommonOperations(presentation)
{
}

int Operations1bpp::getBpp()
{
  return 1;
}

Scroom::Utils::Stuff Operations1bpp::cache(const ConstTile::Ptr tile)
{
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, tile->width);
  unsigned char* data = (unsigned char*)malloc(stride * tile->height);
  Colormap::Ptr colormap = presentation->getColormap();

  unsigned char* row = data;
  for(int j=0; j<tile->height; j++, row+=stride)
  {
    PixelIterator<const byte> bit(tile->data.get()+j*tile->width/8, 0);
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

void Operations1bpp::reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y)
{
  // Reducing by a factor 8. Source tile is 1bpp. Target tile is 8bpp
  int sourceStride = source->width/8;
  const byte* sourceBase = source->data.get();

  int targetStride = target->width;
  byte* targetBase = target->data.get() +
    target->height*y*targetStride/8 +
    target->width*x/8;

  for(int j=0; j<source->height/8;
      j++, targetBase+=targetStride, sourceBase+=sourceStride*8)
  {
    // Iterate vertically over target
    const byte* sourcePtr = sourceBase;
    byte* targetPtr = targetBase;

    for(int i=0; i<source->width/8;
        i++, sourcePtr++, targetPtr++)
    {
      // Iterate horizontally over target

      // Goal is to compute a 8-bit grey value from a 8*8 black/white
      // image. To do so, we take each of the 8 bytes, count the
      // number of 1's in each, and add them. Finally, we divide that
      // by 64 (the maximum number of ones in that area

      const byte* current = sourcePtr;
      int sum = 0;
      for(int k=0; k<8; k++, current+=sourceStride)
        sum += bcl.lookup(*current);

      *targetPtr = sum*255/64;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// Operations8bpp

LayerOperations::Ptr Operations8bpp::create(TiffPresentation* presentation)
{
  return Ptr(new Operations8bpp(presentation));
}

Operations8bpp::Operations8bpp(TiffPresentation* presentation)
  : CommonOperations(presentation)
{
}

int Operations8bpp::getBpp()
{
  return 8;
}

Scroom::Utils::Stuff Operations8bpp::cache(const ConstTile::Ptr tile)
{
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, tile->width);
  unsigned char* data = (unsigned char*)malloc(stride * tile->height);
  Colormap::Ptr colormap = presentation->getColormap();
  const Color& c1 = colormap->colors[0];
  const Color& c2 = colormap->colors[1];

  unsigned char* row = data;
  for(int j=0; j<tile->height; j++, row+=stride)
  {
    const byte* cur = tile->data.get()+j*tile->width;
    
    uint32_t* pixel = (uint32_t*)row;
    for(int i=0; i<tile->width; i++)
    {
      *pixel = mix(c2, c1, 1.0**cur/255).getRGB24();
      
      pixel++;
      ++cur;
    }
  }

  return BitmapSurface::create(tile->width, tile->height, stride, data);
}

void Operations8bpp::reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y)
{
  // Reducing by a factor 8. Source tile is 8bpp. Target tile is 8bpp
  int sourceStride = source->width;
  const byte* sourceBase = source->data.get();

  int targetStride = target->width;
  byte* targetBase = target->data.get() +
    target->height*y*targetStride/8 +
    target->width*x/8;

  for(int j=0; j<source->height/8;
      j++, targetBase+=targetStride, sourceBase+=sourceStride*8)
  {
    // Iterate vertically over target
    const byte* sourcePtr = sourceBase;
    byte* targetPtr = targetBase;

    for(int i=0; i<source->width/8;
        i++, sourcePtr+=8, targetPtr++)
    {
      // Iterate horizontally over target

      // Goal is to compute a 8-bit grey value from a 8*8 grey image.
      const byte* base = sourcePtr;
      int sum = 0;
      for(int k=0; k<8; k++, base+=sourceStride)
      {
        const byte* current=base;
        for(int l=0; l<8; l++, current++)
          sum += *current;
      }

      *targetPtr = sum/64;
    }
  }
}

void Operations8bpp::draw(cairo_t* cr, const ConstTile::Ptr tile,
                      GdkRectangle tileArea, GdkRectangle viewArea, int zoom,
                      Scroom::Utils::Stuff cache)
{
  cairo_save(cr);
  CommonOperations::draw(cr, tile, tileArea, viewArea, zoom, cache);
  cairo_restore(cr);

  // Draw pixelvalues at 32:1 zoom
  if(zoom==5)
  {
    int multiplier = 1<<zoom;
    int stride = tile->width;
    cairo_select_font_face (cr, "Sans",
                            CAIRO_FONT_SLANT_NORMAL,
                            CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 12.0);
    
    for(int x=0; x<tileArea.width; x++)
    {
      for(int y=0; y<tileArea.height; y++)
      {
        const byte* const data = tile->data.get();
        
        int value = data[(tileArea.y+y)*stride + tileArea.x + x];
        
        cairo_save(cr);
        if(value <= 128)
          cairo_set_source_rgb(cr, 1, 1, 1); // White
        else
          cairo_set_source_rgb(cr, 0, 0, 0); // Black
  
        drawPixelValue(cr, viewArea.x+multiplier*x, viewArea.y+multiplier*y, multiplier, value); 
        cairo_restore(cr);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// Operations

LayerOperations::Ptr Operations::create(TiffPresentation* presentation, int bpp)
{
  return Ptr(new Operations(presentation, bpp));
}

Operations::Operations(TiffPresentation* presentation, int bpp)
  : CommonOperations(presentation), 
    bpp(bpp), pixelsPerByte(8/bpp), pixelOffset(bpp), pixelMask((1<<bpp)-1)
{
}

int Operations::getBpp()
{
  return bpp;
}

Scroom::Utils::Stuff Operations::cache(const ConstTile::Ptr tile)
{
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, tile->width);
  unsigned char* data = (unsigned char*)malloc(stride * tile->height);
  Colormap::Ptr colormap = presentation->getColormap();

  unsigned char* row = data;
  for(int j=0; j<tile->height; j++, row+=stride)
  {
    PixelIterator<const byte> pixelIn(tile->data.get()+j*tile->width/pixelsPerByte, 0, bpp);

    uint32_t* pixelOut = (uint32_t*)row;
    for(int i=0; i<tile->width; i++)
    {
      *pixelOut = colormap->colors[*pixelIn].getRGB24();
      pixelOut++;
      ++pixelIn;
    }
  }

  return BitmapSurface::create(tile->width, tile->height, stride, data);
}

void Operations::reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y)
{
  // Reducing by a factor 8. Target is 2*bpp and expects two indices into the colormap
  int sourceStride = source->width/pixelsPerByte;
  const byte* sourceBase = source->data.get();

  const int targetMultiplier = 2; // target is 2*bpp
  int targetStride = targetMultiplier * target->width / pixelsPerByte;
  byte* targetBase = target->data.get() +
    target->height*targetStride*y/8 +
    targetMultiplier*target->width*x/8/pixelsPerByte;

  for(int j=0; j<source->height/8;
      j++, targetBase+=targetStride, sourceBase+=sourceStride*8)
  {
    // Iterate vertically over target
    const byte* sourcePtr = sourceBase;
    PixelIterator<uint16_t> targetPtr((uint16_t*)targetBase, 0, targetMultiplier * bpp);

    for(int i=0; i<source->width/8;
        i++, sourcePtr+=8/pixelsPerByte, ++targetPtr)
    {
      // Iterate horizontally over target

      // Goal is to determine which values occurs most often in a 8*8
      // rectangle, and pick the top two.
      const byte* base = sourcePtr;
      byte lookup[pixelMask+1];
      memset(lookup, 0, sizeof(lookup));

      for(int k=0; k<8; k++, base+=sourceStride)
      {
        PixelIterator<const byte> current(base, 0, bpp);
        for(int l=0; l<8; l++, ++current)
          ++(lookup[*current]);
      }
      unsigned first=0;
      unsigned second=1;
      if(lookup[1]>lookup[0])
      {
        first=1;
        second=0;
      }
      for(unsigned c=2; c<pixelMask+1; c++)
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

void Operations::draw(cairo_t* cr, const ConstTile::Ptr tile,
                      GdkRectangle tileArea, GdkRectangle viewArea, int zoom,
                      Scroom::Utils::Stuff cache)
{
  cairo_save(cr);
  CommonOperations::draw(cr, tile, tileArea, viewArea, zoom, cache);
  cairo_restore(cr);

  // Draw pixelvalues at 32:1 zoom
  if(zoom==5)
  {
    int multiplier = 1<<zoom;
    int stride = tile->width / pixelsPerByte;
    cairo_select_font_face (cr, "Sans",
                            CAIRO_FONT_SLANT_NORMAL,
                            CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size (cr, 12.0);
    
    for(int y=0; y<tileArea.height; y++)
    {
      const byte* const data = tile->data.get();
      PixelIterator<const byte> current(data+(tileArea.y+y)*stride, tileArea.x, bpp);
      
      for(int x=0; x<tileArea.width; x++, ++current)
      {
        int value = *current;
        
        cairo_save(cr);
        if(value <= 8)
          cairo_set_source_rgb(cr, 1, 1, 1); // White
        else
          cairo_set_source_rgb(cr, 0, 0, 0); // Black
  
        drawPixelValue(cr, viewArea.x+multiplier*x, viewArea.y+multiplier*y, multiplier, value); 
        cairo_restore(cr);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// OperationsColormapped

LayerOperations::Ptr OperationsColormapped::create(TiffPresentation* presentation, int bpp)
{
  return Ptr(new OperationsColormapped(presentation, bpp));
}

OperationsColormapped::OperationsColormapped(TiffPresentation* presentation, int bpp)
  : Operations(presentation, bpp)
{
}

int OperationsColormapped::getBpp()
{
  return 2*bpp;
}

Scroom::Utils::Stuff OperationsColormapped::cache(const ConstTile::Ptr tile)
{
  const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, tile->width);
  unsigned char* data = (unsigned char*)malloc(stride * tile->height);
  Colormap::Ptr colormap = presentation->getColormap();
  const int multiplier = 2; // data is 2*bpp, containing 2 colors

  unsigned char* row = data;
  for(int j=0; j<tile->height; j++, row+=stride)
  {
    PixelIterator<uint16_t> pixelIn((uint16_t*)(tile->data.get()+j*multiplier*tile->width/pixelsPerByte), 0, multiplier*bpp);
    uint32_t* pixelOut = (uint32_t*)row;
    for(int i=0; i<tile->width; i++)
    {
      *pixelOut = mix(colormap->colors[*pixelIn & pixelMask], colormap->colors[*pixelIn >> pixelOffset], 0.5).getRGB24();
      
      pixelOut++;
      ++pixelIn;
    }
  }

  return BitmapSurface::create(tile->width, tile->height, stride, data);
}

void OperationsColormapped::reduce(Tile::Ptr target, const ConstTile::Ptr source, int x, int y)
{
  // Reducing by a factor 8. Source and target both 2*bpp, containing 2 colors
  const int multiplier = 2; // data is 2*bpp, containing 2 colors
  int sourceStride = multiplier*source->width/pixelsPerByte;
  const byte* sourceBase = source->data.get();

  int targetStride = multiplier*target->width/pixelsPerByte;
  byte* targetBase = target->data.get() +
    target->height*y*targetStride/8 +
    multiplier*target->width*x/8/pixelsPerByte;

  for(int j=0; j<source->height/8;
      j++, targetBase+=targetStride, sourceBase+=sourceStride*8)
  {
    // Iterate vertically over target
    const byte* sourcePtr = sourceBase;
    PixelIterator<uint16_t> targetPtr((uint16_t*)targetBase, 0, multiplier*bpp);

    for(int i=0; i<source->width/8;
        i++, sourcePtr+=8*multiplier/pixelsPerByte, ++targetPtr)
    {
      // Iterate horizontally over target

      // Goal is to determine which value occurs most often in a 8*8
      // rectangle, and pick that value.
      const byte* base = sourcePtr;
      byte lookup[pixelMask+1];
      memset(lookup, 0, sizeof(lookup));

      for(int k=0; k<8; k++, base+=sourceStride)
      {
        PixelIterator<uint16_t> current((uint16_t*)base, 0, multiplier*bpp);
        for(int l=0; l<8; l++, ++current)
        {
          ++lookup[*current & pixelMask];
          ++lookup[*current >> pixelOffset];
        }
      }
      unsigned first=0;
      unsigned second=1;
      if(lookup[1]>lookup[0])
      {
        first=1;
        second=0;
      }
      for(unsigned c=2; c<pixelMask+1; c++)
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

