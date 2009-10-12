#include "layercoordinator.hh"

#include <workinterface.hh>
#include <threadpool.hh>

#include "local.hh"

class TileReducer : public WorkInterface
{
private:
  LayerOperations* lo;
  TileInternal* targetTile;
  TileInternal* sourceTile;
  int x;
  int y;
  boost::mutex& mut;
  int& unfinishedSourceTiles;
  
public:
  TileReducer(LayerOperations* lo,
              TileInternal* targetTile, TileInternal* sourceTile,
              int x, int y,
              boost::mutex& mut, int& unfinishedSourceTiles);

  // WorkInterface ///////////////////////////////////////////////////////
  virtual bool doWork();
};

////////////////////////////////////////////////////////////////////////

LayerCoordinator::LayerCoordinator(TileInternal* targetTile,
                                   LayerOperations* lo)
  : targetTile(targetTile), lo(lo), unfinishedSourceTiles(0)
{
}

LayerCoordinator::~LayerCoordinator()
{
  std::map<TileInternal*,std::pair<int,int> >::iterator cur =
    sourceTiles.begin();
  std::map<TileInternal*,std::pair<int,int> >::iterator end =
    sourceTiles.end();

  for(;cur!=end;cur++)
  {
    cur->first->unregisterObserver(this);
  }
  sourceTiles.clear();
}

void LayerCoordinator::addSourceTile(int x, int y, TileInternal* tile)
{
  boost::unique_lock<boost::mutex> lock(mut);

  sourceTiles[tile] = std::make_pair(x,y);
  tile->registerObserver(this);
  unfinishedSourceTiles++;
}

////////////////////////////////////////////////////////////////////////
/// TileInternalObserver

void LayerCoordinator::tileFinished(TileInternal* tile)
{
  targetTile->initialize();
  std::pair<int,int> location = sourceTiles[tile];

  TileReducer* tr = new TileReducer(lo, targetTile, tile,
                                    location.first, location.second,
                                    mut, unfinishedSourceTiles);

  schedule(tr, REDUCE_PRIO);
}

////////////////////////////////////////////////////////////////////////
/// TileReducer

TileReducer::TileReducer(LayerOperations* lo,
                         TileInternal* targetTile, TileInternal* sourceTile,
                         int x, int y,
                         boost::mutex& mut, int& unfinishedSourceTiles)
  : lo(lo), targetTile(targetTile), sourceTile(sourceTile),
    x(x), y(y), mut(mut), unfinishedSourceTiles(unfinishedSourceTiles)
{
}

bool TileReducer::doWork()
{
  Tile::Ptr target = targetTile->getTile();
  Tile::Ptr source = sourceTile->getTile();

  lo->reduce(target, source, x, y);

  boost::unique_lock<boost::mutex> lock(mut);
  unfinishedSourceTiles--;
  if(!unfinishedSourceTiles)
  {
    targetTile->reportFinished();
  }
  return false;
}
