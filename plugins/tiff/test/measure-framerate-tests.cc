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

#include "measure-framerate-tests.hh"

#include <time.h>

#include <string>

#include <boost/shared_ptr.hpp>

#include <scroom/unused.h>

#include "measure-framerate-callbacks.hh"
#include "measure-framerate-stubs.hh"


////////////////////////////////////////////////////////////////////////

TestData::Ptr testData;

////////////////////////////////////////////////////////////////////////

class Sleep
{
private:
  unsigned int secs;
  bool started;
  struct timespec t;
public:
  Sleep(unsigned int secs);

  bool operator()();
};

class Invalidator
{
private:
  unsigned int secs;
  bool started;
  struct timespec t;

public:
  Invalidator(unsigned int secs);

  bool operator()();
};

class BaseCounter
{
private:
  std::string name;
  unsigned int secs;
  bool started;
  unsigned int count;
  struct timespec t;

  static unsigned int columnWidth;
  
public:
  BaseCounter(const std::string& name, unsigned int secs);

  bool operator()();
};

class InvalidatingCounter : public BaseCounter
{
public:
  InvalidatingCounter(const std::string& name, unsigned int secs);
  bool operator()();
};

////////////////////////////////////////////////////////////////////////

static bool quit()
{
  gtk_main_quit();

  return false;
}

static bool reset()
{
  testData.reset();

  return false;
}

static bool wait()
{
  if(testData)
    return testData->wait();
  else
    return false;
}

Sleep::Sleep(unsigned int secs)
  : secs(secs), started(false)
{
}

bool Sleep::operator()()
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

Invalidator::Invalidator(unsigned int secs)
  : secs(secs)
{
}

bool Invalidator::operator()()
{
  invalidate();
  
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

unsigned int BaseCounter::columnWidth=0;

BaseCounter::BaseCounter(const std::string& name, unsigned int secs)
  : name(name), secs(secs), started(false), count(0)
{
  columnWidth = std::max(columnWidth, (unsigned int)name.length());
}

bool BaseCounter::operator()()
{
  if(!started && 0==clock_gettime(CLOCK_REALTIME, &t))
  {
    started = true;
    return true;
  }

  count++;
  
  struct timespec now;
  if(0==clock_gettime(CLOCK_REALTIME, &now))
  {
    if(now.tv_sec > t.tv_sec + secs)
    {
      // We're done. Compute frequency.
      double elapsed = (now.tv_nsec - t.tv_nsec)*1e-9;
      elapsed += now.tv_sec - t.tv_sec;
      printf("%-*s: %10.2f Hz\n", columnWidth, name.c_str(), count/elapsed);
      
      return false;
    }
  }

  return true;
}

InvalidatingCounter::InvalidatingCounter(const std::string& name, unsigned int secs)
  : BaseCounter(name, secs)
{
}

bool InvalidatingCounter::operator()()
{
  invalidate();
  return BaseCounter::operator()();
}


////////////////////////////////////////////////////////////////////////

TestData::TestData(TiffPresentation::Ptr tp, const LayerSpec& ls,
                   TiledBitmapInterface::Ptr tbi, SourcePresentation* sp, int zoom)
  : pi(new ProgressInterfaceStub()), vi(new ViewInterfaceStub(pi)), tp(tp), ls(ls), tbi(tbi), sp(sp), zoom(zoom)
{
  tbi->open(vi);
}

TestData::Ptr TestData::create(TiffPresentation::Ptr tp, const LayerSpec& ls,
                               TiledBitmapInterface::Ptr tbi, SourcePresentation* sp, int zoom)
{
  return TestData::Ptr(new TestData(tp, ls, tbi, sp, zoom));
}

bool TestData::wait()
{
  return !pi->isFinished();
}

TestData::~TestData()
{
  tbi->close(vi);
  tbi.reset();
  delete sp;
  while(!ls.empty())
  {
    delete ls.back();
    ls.pop_back();
  }
  tp.reset();
  delete vi;
  delete pi;
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

const int width = 2*4096;
const int height = 2*4096;
const int zoom = -2;

static bool setupTest1bpp()
{
  TiffPresentation::Ptr tp(new TiffPresentation());
  Colormap::Ptr colormap = Colormap::createDefault(2);
  tp->setColormap(colormap);
  
  LayerSpec ls;
  ls.push_back(new Operations1bpp(tp.get()));
  ls.push_back(new Operations8bpp(tp.get()));

  TiledBitmapInterface::Ptr tbi = createTiledBitmap(width, height, ls);
  SourcePresentation* sp = new Source1Bpp();
  tbi->setSource(sp);

  testData = TestData::create(tp, ls, tbi, sp, zoom);
  
  return false;
}
  
static bool setupTest4bpp()
{
  TiffPresentation::Ptr tp(new TiffPresentation());
  Colormap::Ptr colormap = Colormap::createDefault(16);
  tp->setColormap(colormap);
  
  LayerSpec ls;
  ls.push_back(new Operations(tp.get(), 4));
  ls.push_back(new OperationsColormapped(tp.get(), 4));

  TiledBitmapInterface::Ptr tbi = createTiledBitmap(width, height, ls);
  SourcePresentation* sp = new Source4Bpp();
  tbi->setSource(sp);

  testData = TestData::create(tp, ls, tbi, sp, zoom);
  
  return false;
}
  
static bool setupTest8bpp()
{
  TiffPresentation::Ptr tp(new TiffPresentation());
  Colormap::Ptr colormap = Colormap::createDefault(2);
  tp->setColormap(colormap);
  
  LayerSpec ls;
  ls.push_back(new Operations8bpp(tp.get()));

  TiledBitmapInterface::Ptr tbi = createTiledBitmap(width, height, ls);
  SourcePresentation* sp = new Source8Bpp();
  tbi->setSource(sp);

  testData = TestData::create(tp, ls, tbi, sp, zoom);
  
  return false;
}
  
static bool setupTest8bppColormapped()
{
  TiffPresentation::Ptr tp(new TiffPresentation());
  Colormap::Ptr colormap = Colormap::createDefault(256);
  tp->setColormap(colormap);
  
  LayerSpec ls;
  ls.push_back(new Operations(tp.get(), 8));
  ls.push_back(new OperationsColormapped(tp.get(), 8));

  TiledBitmapInterface::Ptr tbi = createTiledBitmap(width, height, ls);
  SourcePresentation* sp = new Source8Bpp();
  tbi->setSource(sp);

  testData = TestData::create(tp, ls, tbi, sp, zoom);
  
  return false;
}
  
////////////////////////////////////////////////////////////////////////

void init_tests()
{
  const unsigned int testDuration = 15;
  const unsigned int sleepDuration = 2;
  functions.push_back(Sleep(sleepDuration));
  functions.push_back(BaseCounter("Baseline (no invalidate)", testDuration));

  functions.push_back(Invalidator(sleepDuration));
  functions.push_back(InvalidatingCounter("Baseline (no redraw)", testDuration));

  functions.push_back(setupTest1bpp);
  functions.push_back(wait);
  functions.push_back(Invalidator(sleepDuration));
  functions.push_back(InvalidatingCounter("1bpp, 1:4 zoom", testDuration));

  functions.push_back(setupTest4bpp);
  functions.push_back(wait);
  functions.push_back(Invalidator(sleepDuration));
  functions.push_back(InvalidatingCounter("4bpp, 1:4 zoom, colormapped", testDuration));

  functions.push_back(setupTest8bpp);
  functions.push_back(wait);
  functions.push_back(Invalidator(sleepDuration));
  functions.push_back(InvalidatingCounter("8bpp, 1:4 zoom", testDuration));

  functions.push_back(setupTest8bppColormapped);
  functions.push_back(wait);
  functions.push_back(Invalidator(sleepDuration));
  functions.push_back(InvalidatingCounter("8bpp, 1:4 zoom, colormapped", testDuration));

  functions.push_back(reset);
  functions.push_back(quit);
}
