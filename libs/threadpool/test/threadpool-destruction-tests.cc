/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/threadpool.hh>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>
#include <memory>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#include <gtest/gtest.h>

#include <scroom/function-additor.hh>
#include <scroom/semaphore.hh>

#include "helpers.hh"

using namespace boost::posix_time;
using namespace Scroom;

[[maybe_unused]]
const millisec short_timeout(250);
const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

TEST(ThreadPool_destruction_Tests, destroy_threadpool_with_nonempty_queue) // NOLINT
{
  ThreadPool::Ptr pool = ThreadPool::create(1);
  Semaphore       guard(0);
  Semaphore       a(0);
  Semaphore       b(0);
  Semaphore       c(0);

  pool->schedule(clear(&a) + pass(&b));
  pool->schedule(clear(&c));

  // Give the thread some time to start the job
  a.P();

  boost::thread t(pass(&guard) + destroy(pool));
  pool.reset();
  guard.V();

  // Thread t destroys the threadpool without waiting for ThreadPool
  // jobs to finish. Hence, it should terminate immediately, even
  // though the threadpool is blocked on pass(&b)
  EXPECT_TRUE(t.timed_join(long_timeout));
  ASSERT_TRUE(boost::thread::id() == t.get_id());
  b.V();
  EXPECT_FALSE(c.P(long_timeout));
}

TEST(ThreadPool_destruction_Tests, destroy_threadpool_with_nonempty_queue_with_completeAllJobsBeforeDestruction_true) // NOLINT
{
  ThreadPool::Ptr pool = ThreadPool::create(1, true);
  Semaphore       guard(0);
  Semaphore       a(0);
  Semaphore       b(0);
  Semaphore       c(0);

  pool->schedule(clear(&a) + pass(&b));
  pool->schedule(clear(&c));

  // Give the thread some time to start the job
  a.P();

  boost::thread t(pass(&guard) + destroy(pool));
  pool.reset();
  guard.V();

  // Thread t destroys the threadpool without waiting for ThreadPool
  // jobs to finish. Hence, it should terminate immediately, even
  // though the threadpool is blocked on pass(&b)
  EXPECT_TRUE(t.timed_join(long_timeout));
  ASSERT_TRUE(boost::thread::id() == t.get_id());
  b.V();
  EXPECT_TRUE(c.P(long_timeout));
}
