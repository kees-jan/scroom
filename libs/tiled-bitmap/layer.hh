#ifndef _LAYER_HH
#define _LAYER_HH

#include <vector>

#include "tileinternal.hh"

class Layer
{
private:
  typedef std::vector<std::vector<TileInternal> > TileGrid;
  typedef std::vector<TileInternal> TileLine;
  
  int depth;
  int width;
  int height;
  int bpp;
  int horTileCount;
  int verTileCount;
  TileGrid tiles;
  TileInternal outOfBounds;
  
public:
  Layer(int depth, int layerWidth, int layerHeight, int bpp);

  TileInternal& getTile(int i, int j);
};


#endif
