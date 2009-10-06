#include "tileinternal.hh"

#include <string.h>

#include <unused.h>

////////////////////////////////////////////////////////////////////////
/// TileInternalObserver

void TileInternalObserver::tileFinished(TileInternal* tile)
{
  UNUSED(tile);
}

////////////////////////////////////////////////////////////////////////
/// TileInternal
TileInternal::TileInternal(int depth, int x, int y, int bpp, TileState state)
  : depth(depth), x(x), y(y), bpp(bpp), state(state), tile(), data(TILESIZE*TILESIZE * bpp / 8)
{
  registerMMI(this, TILESIZE*TILESIZE * bpp / 8, 1);
}

Tile::Ptr TileInternal::getTile()
{
  Tile::Ptr result = tile.lock();
  if(!result)
  {
    // Apparently, tile isn't in use
    boost::unique_lock<boost::mutex> lock(mut);
    result = tile.lock();
    if(!result && (state == TILE_UNLOADED || state == TILE_LOADED)) // Double checked locking
    {
      result = Tile::Ptr(new Tile(TILESIZE, TILESIZE, bpp, data.load()));
      tile = result;
      state = TILE_LOADED;
      loadNotification(this);
    }
  }
  
  return result;
}

void TileInternal::initialize()
{
  boost::unique_lock<boost::mutex> lock(mut);

  if(state == TILE_UNINITIALIZED)
  {
    data.initialize(0);
    state = TILE_LOADED;
    loadNotification(this);
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

bool TileInternal::do_unload()
{
  bool isUnloaded = false;
  Tile::Ptr result = tile.lock();
  if(!result)
  {
    // Apparently, tile isn't in use
    boost::unique_lock<boost::mutex> lock(mut);
    result = tile.lock();
    if(!result && state == TILE_LOADED) // Double checked locking
    {
      // Tile not in use. We can unload now...
      data.unload();
      state = TILE_UNLOADED;
      loadNotification(this);
      isUnloaded=true;
    }
  }

  return isUnloaded;
}
