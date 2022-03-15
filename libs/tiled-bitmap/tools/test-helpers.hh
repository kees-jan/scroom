/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <gtk/gtk.h>

#include <scroom/colormappable.hh>
#include <scroom/layeroperations.hh>

#include "measure-framerate-stubs.hh"

extern int drawingAreaWidth;
extern int drawingAreaHeight;

////////////////////////////////////////////////////////////////////////

class DummyColormapProvider : public ColormapProvider
{
public:
  using Ptr = boost::shared_ptr<DummyColormapProvider>;

private:
  Colormap::Ptr colormap;

private:
  DummyColormapProvider(Colormap::Ptr colormap);

public:
  static Ptr create(Colormap::Ptr colormap);

public:
  Colormap::Ptr getColormap() override;
};

class TestData
{
public:
  using Ptr = boost::shared_ptr<TestData>;

private:
  ProgressInterfaceStub::Ptr pi;
  ViewInterface::Ptr         vi;
  DummyColormapProvider::Ptr colormapProvider;
  LayerSpec                  ls;
  TiledBitmapInterface::Ptr  tbi;
  SourcePresentation::Ptr    sp;
  int                        zoom;

private:
  TestData(DummyColormapProvider::Ptr colormapProvider,
           const LayerSpec&           ls,
           TiledBitmapInterface::Ptr  tbi,
           SourcePresentation::Ptr    sp,
           int                        zoom);

public:
  static Ptr create(DummyColormapProvider::Ptr colormapProvider,
                    const LayerSpec&           ls,
                    TiledBitmapInterface::Ptr  tbi,
                    SourcePresentation::Ptr    sp,
                    int                        zoom);

  ~TestData();
  TestData(const TestData&)           = delete;
  TestData(TestData&&)                = delete;
  TestData operator=(const TestData&) = delete;
  TestData operator=(TestData&&)      = delete;


  void redraw(cairo_t* cr);
  bool wait();
};

extern TestData::Ptr testData;

////////////////////////////////////////////////////////////////////////

class Sleeper
{
private:
  unsigned int    secs;
  bool            started{false};
  struct timespec t = {0, 0};

public:
  explicit Sleeper(unsigned int secs);

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
