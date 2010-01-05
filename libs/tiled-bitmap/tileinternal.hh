#ifndef _TILEINTERNAL_HH
#define _TILEINTERNAL_HH

#include <vector>

#include <boost/thread.hpp>

#include <tiledbitmapinterface.hh>
#include <tile.hh>

#include <observable.hh>


#define TILESIZE 1024

class TileInternal;

class TileInternalObserver
{
public:
  virtual ~TileInternalObserver() {}

  virtual void tileCreated(TileInternal* tile);
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
  boost::mutex mut;
  
public:
  TileInternal(int depth, int x, int y, int bpp, TileState state=TILE_UNINITIALIZED);

  void initialize();
  
  void registerObserver(TileInternalObserver* observer);

  Tile::Ptr getTile();

  void reportFinished();
};

typedef std::vector<TileInternal*> TileInternalLine;
typedef std::vector<TileInternalLine> TileInternalGrid;

#endif
