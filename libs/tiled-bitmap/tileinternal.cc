#include "tileinternal.hh"

TileInternal::TileInternal(int depth, int x, int y, int bpp, TileState state)
  : depth(depth), x(x), y(y), bpp(bpp), state(state)
{
}

Tile TileInternal::getTile()
{
  return Tile(TILESIZE, TILESIZE, bpp, NULL);
}
