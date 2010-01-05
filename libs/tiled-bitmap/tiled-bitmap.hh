#ifndef _TILED_BITMAP_HH
#define _TILED_BITMAP_HH

#include <tiledbitmapinterface.hh>

#include <list>
#include <map>

#include <boost/thread/mutex.hpp>

#include "layer.hh"
#include "layercoordinator.hh"

class TiledBitmapViewData : public ViewIdentifier
{
public:
  ViewInterface* viewInterface;
  GtkProgressBar* progressBar;

public:
  TiledBitmapViewData(ViewInterface* viewInterface);
  virtual ~TiledBitmapViewData();

  void gtk_progress_bar_set_fraction(double fraction);
};

class TiledBitmap : public TiledBitmapInterface, private TileInternalObserver
{
private:
  int bitmapWidth;
  int bitmapHeight;
  LayerSpec ls;
  std::vector<Layer*> layers;
  std::list<LayerCoordinator*> coordinators;
  GtkProgressBar* progressBar;
  boost::mutex viewDataMutex;
  std::map<TiledBitmapViewData*, ViewInterface*> viewData;
  int tileCount;
  boost::mutex tileFinishedMutex;
  int tileFinishedCount;
  
public:
  TiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls);
  virtual ~TiledBitmap();

private:
  void drawTile(cairo_t* cr, const TileInternal* tile, const GdkRectangle viewArea);
  void connect(Layer* layer, Layer* prevLayer, LayerOperations* prevLo);
  void gtk_progress_bar_set_fraction(double fraction);

  ////////////////////////////////////////////////////////////////////////
  // TiledBitmapInterface

public:
  virtual void setSource(SourcePresentation* sp);
  virtual ViewIdentifier* open(ViewInterface* viewInterface);
  virtual void close(ViewIdentifier* vid);
  virtual void redraw(ViewIdentifier* vid, cairo_t* cr, GdkRectangle presentationArea, int zoom);

  ////////////////////////////////////////////////////////////////////////
  // TileInternalObserver

  virtual void tileCreated(TileInternal* tile);
  virtual void tileFinished(TileInternal* tile);
};

#endif
