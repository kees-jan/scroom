/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <gtk/gtk.h>

#include <scroom/layeroperations.hh>
#include <scroom/colormappable.hh>

#include "measure-framerate-stubs.hh"

extern int drawingAreaWidth;
extern int drawingAreaHeight;

////////////////////////////////////////////////////////////////////////

class DummyColormapProvider: public ColormapProvider
{
public:
  typedef boost::shared_ptr<DummyColormapProvider> Ptr;

private:
  Colormap::Ptr colormap;

private:
  DummyColormapProvider(Colormap::Ptr colormap);

public:
  static Ptr create(Colormap::Ptr colormap);

public:
  virtual Colormap::Ptr getColormap();
};

class TestData
{
public:
  typedef boost::shared_ptr<TestData> Ptr;

private:
  ProgressInterfaceStub::Ptr pi;
  ViewInterface::Ptr vi;
  DummyColormapProvider::Ptr colormapProvider;
  LayerSpec ls;
  TiledBitmapInterface::Ptr tbi;
  SourcePresentation::Ptr sp;
  int zoom;

private:
  TestData(DummyColormapProvider::Ptr colormapProvider, const LayerSpec& ls,
           TiledBitmapInterface::Ptr tbi, SourcePresentation::Ptr sp, int zoom);

public:
  static Ptr create(DummyColormapProvider::Ptr colormapProvider, const LayerSpec& ls,
                    TiledBitmapInterface::Ptr tbi, SourcePresentation::Ptr sp, int zoom);

  ~TestData();

  void redraw(cairo_t* cr);
  bool wait();
};

extern TestData::Ptr testData;

////////////////////////////////////////////////////////////////////////

class Sleeper
{
private:
  unsigned int secs;
  bool started;
  struct timespec t;
public:
  Sleeper(unsigned int secs);

  bool operator()();
};

////////////////////////////////////////////////////////////////////////

bool quit();
bool reset();
bool wait();

bool setupTest1bpp(int zoom, int width, int height);
bool setupTest2bpp(int zoom, int width, int height);
bool setupTest4bpp(int zoom, int width, int height);
bool setupTest8bpp(int zoom, int width, int height);
bool setupTest8bppColormapped(int zoom, int width, int height);
