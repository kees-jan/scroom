#include "layer.hh"

#include <stdio.h>

#include <threadpool.hh>

class DataFetcher : public WorkInterface
{
private:
  Layer* layer;
  int width;
  int height;
  int horTileCount;
  int verTileCount;
  int currentRow;
  SourcePresentation* sp;
  
public:
  DataFetcher(Layer* layer,
              int width, int height,
              int horTileCount, int verTileCount,
              SourcePresentation* sp);
  virtual bool doWork();
};

////////////////////////////////////////////////////////////////////////
/// Layer 
Layer::Layer(int depth, int layerWidth, int layerHeight, int bpp)
  : depth(depth), width(layerWidth), height(layerHeight), bpp(bpp)
{
  horTileCount = (width+TILESIZE-1)/TILESIZE;
  verTileCount = (height+TILESIZE-1)/TILESIZE;

  for(int j=0; j<verTileCount; j++)
  {
    tiles.push_back(TileInternalLine());
    TileInternalLine& tl = tiles[j];
    for(int i=0; i<horTileCount; i++)
    {
      tl.push_back(new TileInternal(depth, i, j, bpp));
    }
  }

  outOfBounds = new TileInternal(depth, -1, -1, bpp, TILE_OUT_OF_BOUNDS);
  for(int i=0; i<horTileCount; i++)
  {
    lineOutOfBounds.push_back(outOfBounds);
  }
  
  printf("Layer %d (%d bpp), %d*%d, TileCount %d*%d\n",
         depth, bpp, width, height, horTileCount, verTileCount);
}

int Layer::getHorTileCount()
{
  return horTileCount;
}

int Layer::getVerTileCount()
{
  return verTileCount;
}

TileInternal* Layer::getTile(int i, int j)
{
  if(0<=i && i<horTileCount &&
     0<=j && j<verTileCount)
  {
    return tiles[j][i];
  }
  else
  {
    return outOfBounds;
  }
}

TileInternalLine& Layer::getTileLine(int j)
{
  if(0<=j && j<verTileCount)
  {
    return tiles[j];
  }
  else
  {
    return lineOutOfBounds;
  }
}

void Layer::fetchData(SourcePresentation* sp)
{
  DataFetcher* df = new DataFetcher(this,
                                    width, height,
                                    horTileCount, verTileCount,
                                    sp);
  schedule(df, PRIO_NORMAL);
}

////////////////////////////////////////////////////////////////////////
/// DataFetcher

DataFetcher::DataFetcher(Layer* layer,
                         int width, int height,
                         int horTileCount, int verTileCount,
                         SourcePresentation* sp)
  : layer(layer), width(width), height(height),
    horTileCount(horTileCount), verTileCount(verTileCount),
    currentRow(0), sp(sp)
{
}

bool DataFetcher::doWork()
{
  // printf("Attempting to fetch bitmap data for tileRow %d...\n", currentRow);

  TileInternalLine& tileLine = layer->getTileLine(currentRow);
  std::vector<Tile::Ptr> tiles;
  for(int x = 0; x < horTileCount; x++)
  {
    TileInternal* ti = tileLine[x];
    ti->initialize();
    tiles.push_back(ti->getTile());
  }
  int lineCount = std::min(TILESIZE, height-currentRow*TILESIZE);

  sp->fillTiles(currentRow * TILESIZE, lineCount, TILESIZE, 0, tiles);

  for(int x = 0; x < horTileCount; x++)
  {
    tileLine[x]->reportFinished();
  }
  
  currentRow++;
  return currentRow<verTileCount;
}
