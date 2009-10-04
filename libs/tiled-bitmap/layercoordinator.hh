#ifndef LAYERCOORDINATOR_HH
#define LAYERCOORDINATOR_HH

#include <map>
#include <utility>

#include "tileinternal.hh"

class LayerCoordinator: private TileInternalObserver
{
private:
  TileInternal* targetTile;
  std::map<TileInternal*,std::pair<int,int> > sourceTiles;
  LayerOperations* lo;

public:
  LayerCoordinator(TileInternal* targetTile, LayerOperations* lo);
  virtual ~LayerCoordinator();
  
  void addSourceTile(int x, int y, TileInternal* tile);

private:
  ////////////////////////////////////////////////////////////////////////
  /// TileInternalObserver
  virtual void tileFinished(TileInternal* tile);
};

#endif
