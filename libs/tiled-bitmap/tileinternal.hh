#ifndef _TILEINTERNAL_HH
#define _TILEINTERNAL_HH

#include <vector>

#include <boost/thread.hpp>

#include <tiledbitmapinterface.hh>
#include <tile.hh>

#include <observable.hh>

#include <memorymanagerinterface.hh>


#define TILESIZE 4096
// #define TILESIZE 1024

class TileInternal;

class TileInternalObserver
{
public:
  virtual void tileFinished(TileInternal* tile);
};

class TileInternal : public Observable<TileInternalObserver>, public MemoryManagedInterface
{
public:
  int depth;
  int x;
  int y;
  int bpp;
  TileState state;
  Tile::WeakPtr tile;
  FileBackedMemory data;
  boost::mutex mut;
  
public:
  TileInternal(int depth, int x, int y, int bpp, TileState state=TILE_UNINITIALIZED);

  void initialize();
  
  Tile::Ptr getTile();

  void reportFinished();

  // MemoryManagedInterface //////////////////////////////////////////////
  virtual bool do_unload();
};

typedef std::vector<TileInternal*> TileInternalLine;
typedef std::vector<TileInternalLine> TileInternalGrid;

#endif
