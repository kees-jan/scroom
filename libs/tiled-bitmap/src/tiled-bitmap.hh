/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _TILED_BITMAP_HH
#define _TILED_BITMAP_HH

#include <scroom/scroominterface.hh>
#include <scroom/tiledbitmapinterface.hh>

#include <list>
#include <map>

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <scroom/threadpool.hh>

#include "layer.hh"
#include "layercoordinator.hh"
#include "tiledbitmapviewdata.hh"

class TiledBitmap;

class FileOperation
{
public:
  typedef boost::shared_ptr<FileOperation> Ptr;
private:
  boost::shared_ptr<TiledBitmap> parent;
  boost::mutex waitingMutex;
  bool waiting;
  int timer;

protected:
  FileOperation(boost::shared_ptr<TiledBitmap> parent);

public:
  virtual ~FileOperation() {}

  virtual void doneWaiting();
  virtual bool timerExpired();
  virtual void finished()=0;
  virtual void operator()()=0;
};


class TiledBitmap : public TiledBitmapInterface, public TileInitialisationObserver,
                    public boost::enable_shared_from_this<TiledBitmap>
{
public:
  typedef boost::shared_ptr<TiledBitmap> Ptr;
  typedef boost::weak_ptr<TiledBitmap> WeakPtr;
  
private:
  int bitmapWidth;
  int bitmapHeight;
  LayerSpec ls;
  std::vector<Layer*> layers;
  std::list<LayerCoordinator::Ptr> coordinators;
  GtkProgressBar* progressBar;
  boost::mutex viewDataMutex;
  std::map<ViewInterface*, TiledBitmapViewData::Ptr> viewData;
  int tileCount;
  boost::mutex tileFinishedMutex;
  int tileFinishedCount;
  FileOperation::Ptr fileOperation;
  
public:
  static Ptr create(int bitmapWidth, int bitmapHeight, LayerSpec& ls);
  virtual ~TiledBitmap();

private:
  TiledBitmap(int bitmapWidth, int bitmapHeight, LayerSpec& ls);
  void initialize();
  
private:
  void drawTile(cairo_t* cr, const TileInternal::Ptr tile, const GdkRectangle viewArea);
  void connect(Layer* layer, Layer* prevLayer, LayerOperations* prevLo);
  void gtk_progress_bar_set_fraction(double fraction);

public:
  void gtk_progress_bar_pulse();

  ////////////////////////////////////////////////////////////////////////
  // TiledBitmapInterface

public:
  virtual void setSource(SourcePresentation* sp);
  virtual void open(ViewInterface* viewInterface);
  virtual void close(ViewInterface* vi);
  virtual void redraw(ViewInterface* vi, cairo_t* cr, GdkRectangle presentationArea, int zoom);

  ////////////////////////////////////////////////////////////////////////
  // TileInitialisationObserver

  virtual void tileCreated(TileInternal::Ptr tile);
  virtual void tileFinished(TileInternal::Ptr tile);
};

#endif
