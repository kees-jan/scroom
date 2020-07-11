/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "measure-load-performance-tests.hh"

#include <time.h>

#include <string>

#include <boost/shared_ptr.hpp>

#include <scroom/unused.hh>
#include <scroom/semaphore.hh>
#include <scroom/threadpool.hh>

#include "measure-framerate-callbacks.hh"
#include "measure-framerate-stubs.hh"
#include "test-helpers.hh"

////////////////////////////////////////////////////////////////////////

static void clear(Scroom::Semaphore& s)
{
	s.V();
}

class WaitForAsyncOp
{
private:
  std::string name;
  Scroom::Semaphore s;
  bool started;
  struct timespec t;
public:
  WaitForAsyncOp(const std::string& name);
  WaitForAsyncOp(const WaitForAsyncOp& other);

  bool operator()();
};

WaitForAsyncOp::WaitForAsyncOp(const std::string& name_)
  : name(name_), started(false)
{
}

WaitForAsyncOp::WaitForAsyncOp(const WaitForAsyncOp& other)
: name(other.name), s(), started(other.started), t(other.t)
{
}

bool WaitForAsyncOp::operator()()
{
  if(!started && 0==clock_gettime(CLOCK_REALTIME, &t))
  {
    started = true;

    Sequentially()->schedule(boost::bind(clear, boost::ref(s)));

    std::cout << "Waiting for " << name << std::endl;
    return true;
  }

  s.P();
  struct timespec now;
  if(0==clock_gettime(CLOCK_REALTIME, &now))
  {
    double duration = now.tv_sec - t.tv_sec + (now.tv_nsec - t.tv_nsec) / 1E9;
    
    std::cout << name << " took " << duration << "s" << std::endl;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////

void init_tests()
{
  const int width = 240000;
  const int height = 240000;
  // const unsigned int testDuration = 15;
  const unsigned int sleepDuration = 2;

  functions.push_back(Sleeper(sleepDuration));

  functions.push_back(boost::bind(setupTest1bpp, -2, width, height));
  functions.push_back(WaitForAsyncOp("File load 1bpp"));

  // First run may fill the cache
  functions.push_back(boost::bind(setupTest1bpp, -2, width, height));
  functions.push_back(WaitForAsyncOp("File load 1bpp"));

  functions.push_back(boost::bind(setupTest2bpp, -2, width, height));
  functions.push_back(WaitForAsyncOp("File load 2bpp"));

  functions.push_back(boost::bind(setupTest4bpp, -2, width, height));
  functions.push_back(WaitForAsyncOp("File load 4bpp"));

  functions.push_back(boost::bind(setupTest8bpp, -2, width, height));
  functions.push_back(WaitForAsyncOp("File load 8bpp greyscale"));

  functions.push_back(boost::bind(setupTest8bppColormapped, -2, width, height));
  functions.push_back(WaitForAsyncOp("File load 8bpp colormapped"));

  functions.push_back(reset);
  functions.push_back(quit);
}
