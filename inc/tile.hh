#ifndef _TILE_HH
#define _TILE_HH

typedef unsigned char byte;

class Tile
{
public:
  int width;
  int height;
  int bpp;
  byte* data;

public:
  Tile(int width, int height, int bpp, byte* data)
    : width(width), height(height), bpp(bpp), data(data)
  {}
};

#endif
