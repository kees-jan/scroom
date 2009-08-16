#include "layer.hh"

#include <stdio.h>

Layer::Layer(int depth, int layerWidth, int layerHeight, int bpp)
  : depth(depth), width(layerWidth), height(layerHeight), bpp(bpp)
{
  horTileCount = (width+TILESIZE-1)/TILESIZE;
  verTileCount = (height+TILESIZE-1)/TILESIZE;

  for(int i=0; i<horTileCount; i++)
  {
    tiles.push_back(TileLine());
    TileLine& tl = tiles[i];
    for(int j=0; j<verTileCount; j++)
    {
      tl.push_back(TileInternal(depth, i, j));
    }
  }
  printf("Layer %d (%d bpp), %d*%d, TileCount %d*%d\n",
         depth, bpp, width, height, horTileCount, verTileCount);
}
