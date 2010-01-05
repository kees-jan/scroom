#include "tileinternal.hh"

#include <string.h>

#include <unused.h>

////////////////////////////////////////////////////////////////////////
/// TileInternalObserver

void TileInternalObserver::tileFinished(TileInternal* tile)
{
  UNUSED(tile);
}

void TileInternalObserver::tileCreated(TileInternal* tile)
{
  UNUSED(tile);
}

////////////////////////////////////////////////////////////////////////
/// TileInternal
TileInternal::TileInternal(int depth, int x, int y, int bpp, TileState state)
  : depth(depth), x(x), y(y), bpp(bpp), state(state), tile(), data(NULL)
{
}

Tile::Ptr TileInternal::getTile()
{
  boost::unique_lock<boost::mutex> lock(mut);

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
  boost::unique_lock<boost::mutex> lock(mut);

  if(state == TILE_UNINITIALIZED)
  {
    int dataSize = TILESIZE*TILESIZE * bpp / 8;

    data = (byte*)malloc(dataSize*sizeof(byte));
    memset(data, 0, dataSize*sizeof(byte));
    state = TILE_LOADED;
  }
}
  
void TileInternal::reportFinished()
{
  std::list<TileInternalObserver*> observers = getObservers();
  while(!observers.empty())
  {
    observers.front()->tileFinished(this);
    observers.pop_front();
  }
}

void TileInternal::registerObserver(TileInternalObserver* observer)
{
  Observable<TileInternalObserver>::registerObserver(observer);
  observer->tileCreated(this);
}
