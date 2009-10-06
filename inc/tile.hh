#ifndef _TILE_HH
#define _TILE_HH

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <global.hh>

class Tile
{
public:
  typedef boost::shared_ptr<Tile> Ptr;
  typedef boost::weak_ptr<Tile> WeakPtr;
  
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
