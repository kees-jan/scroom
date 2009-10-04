#include "layeroperations.hh"

#include <stdio.h>

////////////////////////////////////////////////////////////////////////
// BitIterator

class BitIterator
{
private:
  byte* currentByte;
  byte currentBit;

public:
  BitIterator();
  BitIterator(byte* base, int offset=0);
  byte get();
  BitIterator& operator++();
  BitIterator operator++(int);
  BitIterator& operator+=(int x);
  byte operator*();
};

BitIterator::BitIterator()
  : currentByte(NULL), currentBit(0)
{
}

BitIterator::BitIterator(byte* base, int offset)
{
  div_t d = div(offset, 8);
  currentByte = base+d.quot;
  currentBit = 7-d.rem;
}

inline byte BitIterator::get()
{
  return (*currentByte>>currentBit) & 1;
}

inline byte BitIterator::operator*()
{
  return (*currentByte>>currentBit) & 1;
}

inline BitIterator& BitIterator::operator++()
{
  // Prefix operator
  if(!(currentBit--))
  {
    currentBit=7;
    ++currentByte;
  }
  
  return *this;
}

inline BitIterator BitIterator::operator++(int)
{
  // Postfix operator
  BitIterator result = *this;
  
  if(!(currentBit--))
  {
    currentBit=7;
    ++currentByte;
  }
  
  return result;
}

BitIterator& BitIterator::operator+=(int x)
{
  int offset = 7-currentBit+x;
  div_t d = div(offset, 8);
  currentByte += d.quot;
  currentBit = 7-d.rem;

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
      BitIterator bit(tile->data+(tileArea.y+j)*tile->width/8, tileArea.x);
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
      BitIterator b(tile->data+(tileArea.y+j*pixelCount)*tile->width/8,
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
  printf("Draw8bpp\n");
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
