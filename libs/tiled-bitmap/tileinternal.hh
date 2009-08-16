#ifndef _TILEINTERNAL_HH
#define _TILEINTERNAL_HH

#include <tiledbitmapinterface.hh>
#include <tile.hh>

#define TILESIZE 1024

class TileInternal
{
public:
  int depth;
  int x;
  int y;
  int bpp;
  TileState state;
  
public:
  TileInternal(int depth, int x, int y, int bpp, TileState state=TILE_UNINITIALIZED);

  Tile getTile();
  
};


#endif
