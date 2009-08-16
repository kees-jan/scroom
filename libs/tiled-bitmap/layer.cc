#include "layer.hh"

#include <stdio.h>

Layer::Layer(int depth, int layerWidth, int layerHeight, int bpp)
  : depth(depth), width(layerWidth), height(layerHeight), bpp(bpp),
    outOfBounds(depth, -1, -1, bpp, TILE_OUT_OF_BOUNDS)
{
  horTileCount = (width+TILESIZE-1)/TILESIZE;
  verTileCount = (height+TILESIZE-1)/TILESIZE;

  for(int i=0; i<horTileCount; i++)
  {
    tiles.push_back(TileLine());
    TileLine& tl = tiles[i];
    for(int j=0; j<verTileCount; j++)
    {
      tl.push_back(TileInternal(depth, i, j, bpp));
    }
  }
  printf("Layer %d (%d bpp), %d*%d, TileCount %d*%d\n",
         depth, bpp, width, height, horTileCount, verTileCount);
}

TileInternal& Layer::getTile(int i, int j)
{
  if(0<=i && i<horTileCount &&
     0<=j && j<verTileCount)
  {
    return tiles[i][j];
  }
  else
  {
    return outOfBounds;
  }
}
