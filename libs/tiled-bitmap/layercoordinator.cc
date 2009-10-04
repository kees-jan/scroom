#include "layercoordinator.hh"

LayerCoordinator::LayerCoordinator(TileInternal* targetTile,
                                   LayerOperations* lo)
  : targetTile(targetTile), lo(lo)
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
  sourceTiles[tile] = std::make_pair(x,y);
  tile->registerObserver(this);
}

////////////////////////////////////////////////////////////////////////
/// TileInternalObserver

void LayerCoordinator::tileFinished(TileInternal* tile)
{
  printf("Received a \"tileFinished\" message!\n");
}
