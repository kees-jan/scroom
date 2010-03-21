#include "layeroperations.hh"

#include <stdio.h>
#include <string.h>

#include "tiffpresentation.hh"

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
// CommonOperations

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

inline void CommonOperations::drawPixel(cairo_t* cr, int x, int y, int size, double greyShade)
{
  cairo_set_source_rgb(cr, greyShade, greyShade, greyShade);
  cairo_move_to(cr, x, y);
  cairo_line_to(cr, x+size, y);
  cairo_line_to(cr, x+size, y+size);
  cairo_line_to(cr, x, y+size);
  cairo_line_to(cr, x, y);
  cairo_fill(cr);
}

inline void CommonOperations::drawPixel(cairo_t* cr, int x, int y, int size, byte greyShade)
{
  drawPixel(cr, x, y, size, (double)(255-greyShade)/255.0);
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
  

////////////////////////////////////////////////////////////////////////
// Operations1bpp

int Operations1bpp::getBpp()
{
  return 1;
}

void Operations1bpp::draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom)
{
  cairo_set_source_rgb(cr, 1, 1, 1); // White
  fillRect(cr, viewArea);
  
  if(zoom>=0)
  {
    // Iterate over pixels in the tileArea, drawing each as we go ahead
    int pixelSize = 1<<zoom;

    for(int j=0; j<tileArea.height; j++)
    {
      PixelIterator bit(tile->data+(tileArea.y+j)*tile->width/8, tileArea.x);
      for(int i=0; i<tileArea.width; i++)
      {
        if(*bit)
          drawPixel(cr, viewArea.x+i*pixelSize, viewArea.y+j*pixelSize, pixelSize);
        ++bit;
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
      PixelIterator b(tile->data+(tileArea.y+j*pixelCount)*tile->width/8,
                    tileArea.x);
      
      for(int i=0; i<viewArea.width; i++)
      {
        if(*b)
        {
          drawPixel(cr, viewArea.x+i, viewArea.y+j, 1);
        }
        b+=pixelCount;
      }
    }
  }
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

int Operations8bpp::getBpp()
{
  return 8;
}

void Operations8bpp::draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom)
{
  cairo_set_source_rgb(cr, 1, 1, 1); // White
  fillRect(cr, viewArea);
  
  if(zoom>=0)
  {
    // Iterate over pixels in the tileArea, drawing each as we go ahead
    int pixelSize = 1<<zoom;

    for(int j=0; j<tileArea.height; j++)
    {
      byte* cur = tile->data+(tileArea.y+j)*tile->width + tileArea.x;
    
      for(int i=0; i<tileArea.width; i++, cur++)
      {
        if(*cur)
          drawPixel(cr, viewArea.x+i*pixelSize, viewArea.y+j*pixelSize,
                    pixelSize, *cur);
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
        if(*cur)
        {
          drawPixel(cr, viewArea.x+i, viewArea.y+j, 1, *cur);
        }
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
  :bpp(bpp), pixelsPerByte(8/bpp), pixelOffset(bpp), pixelMask((1<<bpp)-1), presentation(presentation)
{
}

int Operations::getBpp()
{
  return bpp;
}

void Operations::draw(cairo_t* cr, Tile::Ptr tile, GdkRectangle tileArea, GdkRectangle viewArea, int zoom)
{
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
        if(*pixel)
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
        if(*pixel)
        {
          drawPixel(cr, viewArea.x+i, viewArea.y+j, 1, colormap->colors[*pixel]);
        }
        pixel+=pixelCount;
      }
    }
  }
}

void Operations::reduce(Tile::Ptr target, const Tile::Ptr source, int x, int y)
{
  // Reducing by a factor 8. Source and target have same bpp
  int sourceStride = source->width/pixelsPerByte;
  byte* sourceBase = source->data;

  int targetStride = target->width/pixelsPerByte;
  byte* targetBase = target->data +
    target->height*y*targetStride/8 +
    target->width*x/8/pixelsPerByte;

  for(int j=0; j<source->height/8;
      j++, targetBase+=targetStride, sourceBase+=sourceStride*8)
  {
    // Iterate vertically over target
    byte* sourcePtr = sourceBase;
    PixelIterator targetPtr(targetBase, 0, bpp);

    for(int i=0; i<source->width/8;
        i++, sourcePtr+=8, ++targetPtr)
    {
      // Iterate horizontally over target

      // Goal is to determine which value occurs most often in a 8*8
      // rectangle, and pick that value.
      byte* base = sourcePtr;
      byte lookup[pixelMask+1];
      memset(lookup, 0, sizeof(lookup));
      byte maxValue=0;
      byte maxCount;

      for(int k=0; k<8; k++, base+=sourceStride)
      {
        PixelIterator current(base, 0, bpp);
        for(int l=0; l<8; l++, ++current)
        {
          if(++(lookup[*current])>maxCount)
          {
            maxCount=lookup[*current];
            maxValue=*current;
          }
        }
      }

      targetPtr.set(maxValue);
    }
  }
}

