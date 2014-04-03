/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

#include <measure-load-performance-tests.hh>

#include <time.h>

#include <string>

#include <boost/shared_ptr.hpp>

#include <scroom/unused.h>
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

WaitForAsyncOp::WaitForAsyncOp(const std::string& name)
  : name(name), started(false)
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

    std::cout << "Waiting for " << name;
    return true;
  }

  s.P();
  struct timespec now;
  if(0==clock_gettime(CLOCK_REALTIME, &now))
  {
	  double duration = now.tv_sec - t.tv_sec +
			  (now.tv_nsec - t.tv_nsec) / 1E9;

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

  functions.push_back(Sleep(sleepDuration));
  // functions.push_back(logSizes);
  // functions.push_back(BaseCounter("Baseline (no invalidate)", testDuration));
  // 
  // functions.push_back(Invalidator(sleepDuration));
  // functions.push_back(InvalidatingCounter("Baseline (no redraw)", testDuration));
  // 
  functions.push_back(boost::bind(setupTest1bpp, -2, width, height));
  functions.push_back(WaitForAsyncOp("File load 1bpp"));

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
