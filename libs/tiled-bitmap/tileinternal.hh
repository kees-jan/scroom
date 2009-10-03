#ifndef _TILEINTERNAL_HH
#define _TILEINTERNAL_HH

#include <boost/shared_ptr.hpp>

#include <tiledbitmapinterface.hh>
#include <tile.hh>

#include <observable.hh>


#define TILESIZE 1024

class TileInternal;

class TileInternalObserver
{
public:
  virtual void tileFinished(TileInternal* tile);
};

class TileInternal : public Observable<TileInternalObserver>
{
public:
  int depth;
  int x;
  int y;
  int bpp;
  TileState state;
  Tile::WeakPtr tile;
  byte* data;
  
public:
  TileInternal(int depth, int x, int y, int bpp, TileState state=TILE_UNINITIALIZED);

  void initialize();
  
  Tile::Ptr getTile();

  void reportFinished();
};


#endif
