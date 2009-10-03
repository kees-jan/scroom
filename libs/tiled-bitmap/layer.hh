#ifndef _LAYER_HH
#define _LAYER_HH

#include <vector>

#include <tiledbitmapinterface.hh>

#include "tileinternal.hh"

class Layer
{
public:
  typedef std::vector<TileInternal*> TileLine;
  typedef std::vector<TileLine> TileGrid;
  
private:
  int depth;
  int width;
  int height;
  int bpp;
  int horTileCount;
  int verTileCount;
  TileGrid tiles;
  TileInternal* outOfBounds;
  TileLine lineOutOfBounds;
  
public:
  Layer(int depth, int layerWidth, int layerHeight, int bpp);

  TileInternal* getTile(int i, int j);
  TileLine& getTileLine(int j);
  void fetchData(SourcePresentation* sp);
};


#endif
