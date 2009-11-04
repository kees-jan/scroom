#ifndef _LAYER_HH
#define _LAYER_HH

#include <vector>

#include <tiledbitmapinterface.hh>

#include "tileinternal.hh"

class Layer
{
private:
  int depth;
  int width;
  int height;
  int bpp;
  int horTileCount;
  int verTileCount;
  TileInternalGrid tiles;
  TileInternal* outOfBounds;
  TileInternalLine lineOutOfBounds;
  
public:
  Layer(int depth, int layerWidth, int layerHeight, int bpp);
  int getHorTileCount();
  int getVerTileCount();

  TileInternal* getTile(int i, int j);
  TileInternalLine& getTileLine(int j);
  void fetchData(SourcePresentation* sp);

public:
  int getWidth()
  { return width; }

  int getHeight()
  { return height; }
    
};


#endif
