#include "tileinternal.hh"

#include <string.h>

TileInternal::TileInternal(int depth, int x, int y, int bpp, TileState state)
  : depth(depth), x(x), y(y), bpp(bpp), state(state), tile(), data(NULL)
{
}

Tile::Ptr TileInternal::getTile()
{
  if(state!=TILE_LOADED)
    printf("ERROR: Tile not loaded\n");
  
  Tile::Ptr result = tile.lock();
  if(!result)
  {
    if(!data)
      printf("ERROR: Tile data not available\n");
    result = Tile::Ptr(new Tile(TILESIZE, TILESIZE, bpp, data));
    tile = result;
  }
  
  return result;
}

void TileInternal::initialize()
{
  Tile::Ptr t = tile.lock();
  if(state != TILE_UNINITIALIZED)
    printf("ERROR: Tile already initialized!\n");
  if(t)
    printf("ERROR: Tile already in use\n");

  int dataSize = TILESIZE*TILESIZE * bpp / 8;

  data = (byte*)malloc(dataSize*sizeof(byte));
  memset(data, 0, dataSize*sizeof(byte));
  state = TILE_LOADED;
  t.reset();
}
  
