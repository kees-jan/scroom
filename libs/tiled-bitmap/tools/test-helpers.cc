/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "test-helpers.hh"

////////////////////////////////////////////////////////////////////////

int drawingAreaWidth=0;
int drawingAreaHeight=0;

TestData::Ptr testData;

////////////////////////////////////////////////////////////////////////

DummyColormapProvider::DummyColormapProvider(Colormap::Ptr colormap_)
  : colormap(colormap_)
{}

DummyColormapProvider::Ptr DummyColormapProvider::create(Colormap::Ptr colormap)
{
  return Ptr(new DummyColormapProvider(colormap));
}

Colormap::Ptr DummyColormapProvider::getColormap()
{
  return colormap;
}

////////////////////////////////////////////////////////////////////////

TestData::TestData(DummyColormapProvider::Ptr colormapProvider_, const LayerSpec& ls_,
                   TiledBitmapInterface::Ptr tbi_, SourcePresentation::Ptr sp_, int zoom_)
  : pi(ProgressInterfaceStub::create()), vi(ViewInterfaceStub::create(pi)), colormapProvider(colormapProvider_), ls(ls_), tbi(tbi_), sp(sp_), zoom(zoom_)
{
  tbi_->open(vi);
}

TestData::Ptr TestData::create(DummyColormapProvider::Ptr colormapProvider, const LayerSpec& ls,
                               TiledBitmapInterface::Ptr tbi, SourcePresentation::Ptr sp, int zoom)
{
  return TestData::Ptr(new TestData(colormapProvider, ls, tbi, sp, zoom));
}

bool TestData::wait()
{
  return !pi->isFinished();
}

TestData::~TestData()
{
  tbi->close(vi);
  tbi.reset();
  sp.reset();
  ls.clear();
  colormapProvider.reset();
  vi.reset();
}

void TestData::redraw(cairo_t* cr)
{
  if(tbi)
  {
    GdkRectangle rect;
    rect.x=0;
    rect.y=0;
    if(zoom>=0)
    {
      // Zooming in. Smallest step is 1 presentation pixel, which is more than one window-pixel
      int pixelSize = 1<<zoom;
      rect.width = (drawingAreaWidth+pixelSize-1)/pixelSize;
      rect.height = (drawingAreaHeight+pixelSize-1)/pixelSize;
    }
    else
    {
      // Zooming out. Smallest step is 1 window-pixel, which is more than one presentation-pixel
      int pixelSize = 1<<(-zoom);
      rect.width = drawingAreaWidth*pixelSize;
      rect.height = drawingAreaHeight*pixelSize;
    }

    tbi->redraw(vi, cr, rect, zoom);
  }
}

////////////////////////////////////////////////////////////////////////

Sleeper::Sleeper(unsigned int secs_)
  : secs(secs_), started(false)
{
}

bool Sleeper::operator()()
{
  if(!started && 0==clock_gettime(CLOCK_REALTIME, &t))
  {
    started = true;
    return true;
  }

  struct timespec now;
  if(0==clock_gettime(CLOCK_REALTIME, &now))
  {
    if(now.tv_sec > t.tv_sec + secs)
      return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////

bool quit()
{
  gtk_main_quit();

  return false;
}

bool reset()
{
  testData.reset();

  return false;
}

bool wait()
{
  if(testData)
    return testData->wait();
  else
    return false;
}

////////////////////////////////////////////////////////////////////////

bool setupTest1bpp(int zoom, int width, int height)
{
  Colormap::Ptr colormap = Colormap::createDefault(2);
  DummyColormapProvider::Ptr colormapProvider = DummyColormapProvider::create(colormap);

  LayerSpec ls;
  ls.push_back(Operations1bpp::create(colormapProvider));
  ls.push_back(Operations8bpp::create(colormapProvider));

  TiledBitmapInterface::Ptr tbi = createTiledBitmap(width, height, ls);
  SourcePresentation::Ptr sp(new Source1Bpp());
  tbi->setSource(sp);

  testData = TestData::create(colormapProvider, ls, tbi, sp, zoom);

  return false;
}

bool setupTest2bpp(int zoom, int width, int height)
{
  Colormap::Ptr colormap = Colormap::createDefault(16);
  DummyColormapProvider::Ptr colormapProvider = DummyColormapProvider::create(colormap);

  LayerSpec ls;
  ls.push_back(Operations::create(colormapProvider, 2));
  ls.push_back(OperationsColormapped::create(colormapProvider, 2));

  TiledBitmapInterface::Ptr tbi = createTiledBitmap(width, height, ls);
  SourcePresentation::Ptr sp(new Source2Bpp());
  tbi->setSource(sp);

  testData = TestData::create(colormapProvider, ls, tbi, sp, zoom);

  return false;
}

bool setupTest4bpp(int zoom, int width, int height)
{
  Colormap::Ptr colormap = Colormap::createDefault(16);
  DummyColormapProvider::Ptr colormapProvider = DummyColormapProvider::create(colormap);

  LayerSpec ls;
  ls.push_back(Operations::create(colormapProvider, 4));
  ls.push_back(OperationsColormapped::create(colormapProvider, 4));

  TiledBitmapInterface::Ptr tbi = createTiledBitmap(width, height, ls);
  SourcePresentation::Ptr sp(new Source4Bpp());
  tbi->setSource(sp);

  testData = TestData::create(colormapProvider, ls, tbi, sp, zoom);

  return false;
}

bool setupTest8bpp(int zoom, int width, int height)
{
  Colormap::Ptr colormap = Colormap::createDefault(2);
  DummyColormapProvider::Ptr colormapProvider = DummyColormapProvider::create(colormap);

  LayerSpec ls;
  ls.push_back(Operations8bpp::create(colormapProvider));

  TiledBitmapInterface::Ptr tbi = createTiledBitmap(width, height, ls);
  SourcePresentation::Ptr sp(new Source8Bpp());
  tbi->setSource(sp);

  testData = TestData::create(colormapProvider, ls, tbi, sp, zoom);

  return false;
}

bool setupTest8bppColormapped(int zoom, int width, int height)
{
  Colormap::Ptr colormap = Colormap::createDefault(256);
  DummyColormapProvider::Ptr colormapProvider = DummyColormapProvider::create(colormap);

  LayerSpec ls;
  ls.push_back(Operations::create(colormapProvider, 8));
  ls.push_back(OperationsColormapped::create(colormapProvider, 8));

  TiledBitmapInterface::Ptr tbi = createTiledBitmap(width, height, ls);
  SourcePresentation::Ptr sp(new Source8Bpp());
  tbi->setSource(sp);

  testData = TestData::create(colormapProvider, ls, tbi, sp, zoom);

  return false;
}
