/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <iostream>
#include <memory>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#include <gtest/gtest.h>

#include <scroom/async-deleter.hh>
#include <scroom/function-additor.hh>
#include <scroom/semaphore.hh>

#include "helpers.hh"

using namespace boost::posix_time;
using namespace Scroom;

static const millisec short_timeout(250);
static const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

class A
{
private:
  Semaphore* s1;
  Semaphore* s2;

public:
  A(Semaphore* s1_, Semaphore* s2_)
    : s1(s1_)
    , s2(s2_)
  {
  }
  ~A()
  {
    s1->P();
    s2->V();
  }

  A(const A&) = delete;
  A(A&&) = delete;
  A& operator=(const A&) = delete;
  A& operator=(A&&) = delete;
};

//////////////////////////////////////////////////////////////

TEST(Async_Deleter_Tests, deleter_deletes_asynchronously) // NOLINT
{
  Semaphore barrier1;
  Semaphore destroyed;
  std::shared_ptr<A> a = std::shared_ptr<A>(new A(&barrier1, &destroyed), AsyncDeleter<A>());
  EXPECT_FALSE(destroyed.P(short_timeout));

  Semaphore barrier2;
  Semaphore signal;
  CpuBound()->schedule(pass(&barrier2) + destroy(a) + clear(&signal));
  a.reset();
  barrier2.V();
  EXPECT_TRUE(signal.P(long_timeout));
  EXPECT_FALSE(destroyed.P(short_timeout));
  barrier1.V();
  EXPECT_TRUE(destroyed.P(long_timeout));
}
