/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "measure-load-performance-tests.hh"

#include <ctime>
#include <memory>
#include <string>
#include <utility>

#include <scroom/semaphore.hh>
#include <scroom/threadpool.hh>

#include "measure-framerate-callbacks.hh"
#include "measure-framerate-stubs.hh"
#include "test-helpers.hh"

////////////////////////////////////////////////////////////////////////

class WaitForAsyncOp
{
private:
  std::string       name;
  Scroom::Semaphore s;
  bool              started{false};
  struct timespec   t = {0, 0};

public:
  explicit WaitForAsyncOp(std::string name);
  WaitForAsyncOp(const WaitForAsyncOp& other);
  WaitForAsyncOp(WaitForAsyncOp&& /*other*/);
  WaitForAsyncOp operator=(const WaitForAsyncOp&) = delete;
  WaitForAsyncOp operator=(WaitForAsyncOp&&)      = delete;
  ~WaitForAsyncOp()                               = default;

  bool operator()();
};

WaitForAsyncOp::WaitForAsyncOp(std::string name_)
  : name(std::move(name_))
{
}

WaitForAsyncOp::WaitForAsyncOp(const WaitForAsyncOp& other)
  : name(other.name)
  , started(other.started)
  , t(other.t)
{
}

WaitForAsyncOp::WaitForAsyncOp(WaitForAsyncOp&& other)
  : name(std::move(other.name))
  , started(std::move(other.started))
  , t(std::move(other.t))
{
}

bool WaitForAsyncOp::operator()()
{
  if(!started && 0 == clock_gettime(CLOCK_REALTIME, &t))
  {
    started = true;

    Sequentially()->schedule([&s = this->s] { s.V(); });

    std::cout << "Waiting for " << name << std::endl;
    return true;
  }

  s.P();
  struct timespec now = {0, 0};
  if(0 == clock_gettime(CLOCK_REALTIME, &now))
  {
    const double duration = now.tv_sec - t.tv_sec + (now.tv_nsec - t.tv_nsec) / 1E9;

    std::cout << name << " took " << duration << "s" << std::endl;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////

void init_tests()
{
  const int width  = 240000;
  const int height = 240000;
  // const unsigned int testDuration = 15;
  const unsigned int sleepDuration = 2;

  functions.emplace_back(Sleeper(sleepDuration));

  functions.emplace_back([] { return setupTest1bpp(-2, width, height); });
  functions.emplace_back(WaitForAsyncOp("File load 1bpp"));

  // First run may fill the cache
  functions.emplace_back([] { return setupTest1bpp(-2, width, height); });
  functions.emplace_back(WaitForAsyncOp("File load 1bpp"));

  functions.emplace_back([] { return setupTest2bpp(-2, width, height); });
  functions.emplace_back(WaitForAsyncOp("File load 2bpp"));

  functions.emplace_back([] { return setupTest4bpp(-2, width, height); });
  functions.emplace_back(WaitForAsyncOp("File load 4bpp"));

  functions.emplace_back([] { return setupTest8bpp(-2, width, height); });
  functions.emplace_back(WaitForAsyncOp("File load 8bpp greyscale"));

  functions.emplace_back([] { return setupTest8bppColormapped(-2, width, height); });
  functions.emplace_back(WaitForAsyncOp("File load 8bpp colormapped"));

  functions.emplace_back(reset);
  functions.emplace_back(quit);
}
