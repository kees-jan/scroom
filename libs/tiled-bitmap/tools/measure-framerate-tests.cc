/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "measure-framerate-tests.hh"

#include <ctime>
#include <memory>
#include <string>

#include <spdlog/spdlog.h>

#include <scroom/unused.hh>

#include "measure-framerate-callbacks.hh"
#include "measure-framerate-stubs.hh"
#include "test-helpers.hh"

////////////////////////////////////////////////////////////////////////

class Invalidator
{
private:
  unsigned int    secs;
  bool            started{false};
  struct timespec t = {0, 0};

public:
  explicit Invalidator(unsigned int secs);

  bool operator()();
};

class BaseCounter
{
private:
  std::string     name;
  unsigned int    secs;
  bool            started{false};
  unsigned int    count{0};
  struct timespec t = {0, 0};

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

static bool logSizes()
{
  spdlog::info("Canvas size: {}x{}", drawingAreaWidth, drawingAreaHeight);
  return false;
}

////////////////////////////////////////////////////////////////////////

Invalidator::Invalidator(unsigned int secs_)
  : secs(secs_)
{
}

bool Invalidator::operator()()
{
  invalidate();

  if(!started && 0 == clock_gettime(CLOCK_REALTIME, &t))
  {
    started = true;
    return true;
  }

  struct timespec now = {0, 0};
  if(0 == clock_gettime(CLOCK_REALTIME, &now))
  {
    if(now.tv_sec > t.tv_sec + secs)
    {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////

unsigned int BaseCounter::columnWidth = 0;

BaseCounter::BaseCounter(const std::string& name_, unsigned int secs_)
  : name(name_)
  , secs(secs_)
{
  columnWidth = std::max(columnWidth, static_cast<unsigned int>(name_.length()));
}

bool BaseCounter::operator()()
{
  if(!started && 0 == clock_gettime(CLOCK_REALTIME, &t))
  {
    started = true;
    return true;
  }

  count++;

  struct timespec now = {0, 0};
  if(0 == clock_gettime(CLOCK_REALTIME, &now))
  {
    if(now.tv_sec > t.tv_sec + secs)
    {
      // We're done. Compute frequency.
      double elapsed = now.tv_nsec - t.tv_nsec * 1e-9;
      elapsed += now.tv_sec - t.tv_sec;
      spdlog::info("{:{}}: {:10.2} Hz", name, columnWidth, count / elapsed);

      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////

InvalidatingCounter::InvalidatingCounter(const std::string& name_, unsigned int secs_)
  : BaseCounter(name_, secs_)
{
}

bool InvalidatingCounter::operator()()
{
  invalidate();
  return BaseCounter::operator()();
}

////////////////////////////////////////////////////////////////////////

void init_tests()
{
  const int          width         = 2 * 4096;
  const int          height        = 2 * 4096;
  const unsigned int testDuration  = 15;
  const unsigned int sleepDuration = 2;

  functions.emplace_back(Sleeper(sleepDuration));
  functions.emplace_back(logSizes);
  functions.emplace_back(BaseCounter("Baseline (no invalidate)", testDuration));

  functions.emplace_back(Invalidator(sleepDuration));
  functions.emplace_back(InvalidatingCounter("Baseline (no redraw)", testDuration));

  functions.emplace_back([] { return setupTest1bpp(-2, width, height); });
  functions.emplace_back(wait);
  functions.emplace_back(Invalidator(sleepDuration));
  functions.emplace_back(InvalidatingCounter("1bpp, 1:4 zoom", testDuration));

  functions.emplace_back([] { return setupTest1bpp(4, width, height); });
  functions.emplace_back(wait);
  functions.emplace_back(Invalidator(sleepDuration));
  functions.emplace_back(InvalidatingCounter("1bpp, 16:1 zoom", testDuration));

  functions.emplace_back([] { return setupTest4bpp(-2, width, height); });
  functions.emplace_back(wait);
  functions.emplace_back(Invalidator(sleepDuration));
  functions.emplace_back(InvalidatingCounter("4bpp, 1:4 zoom, colormapped", testDuration));

  functions.emplace_back([] { return setupTest4bpp(4, width, height); });
  functions.emplace_back(wait);
  functions.emplace_back(Invalidator(sleepDuration));
  functions.emplace_back(InvalidatingCounter("4bpp, 16:1 zoom, colormapped", testDuration));

  functions.emplace_back([] { return setupTest8bpp(-2, width, height); });
  functions.emplace_back(wait);
  functions.emplace_back(Invalidator(sleepDuration));
  functions.emplace_back(InvalidatingCounter("8bpp, 1:4 zoom", testDuration));

  functions.emplace_back([] { return setupTest8bpp(4, width, height); });
  functions.emplace_back(wait);
  functions.emplace_back(Invalidator(sleepDuration));
  functions.emplace_back(InvalidatingCounter("8bpp, 16:1 zoom", testDuration));

  functions.emplace_back([] { return setupTest8bppColormapped(-2, width, height); });
  functions.emplace_back(wait);
  functions.emplace_back(Invalidator(sleepDuration));
  functions.emplace_back(InvalidatingCounter("8bpp, 1:4 zoom, colormapped", testDuration));

  functions.emplace_back([] { return setupTest8bppColormapped(4, width, height); });
  functions.emplace_back(wait);
  functions.emplace_back(Invalidator(sleepDuration));
  functions.emplace_back(InvalidatingCounter("8bpp, 16:1 zoom, colormapped", testDuration));

  functions.emplace_back(reset);
  functions.emplace_back(quit);
}
