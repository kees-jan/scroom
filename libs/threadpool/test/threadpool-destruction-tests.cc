/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
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
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <scroom/function-additor.hh>
#include <scroom/semaphore.hh>

#include "helpers.hh"

using namespace boost::posix_time;
using namespace Scroom;

const millisec short_timeout(250);
const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(ThreadPool_destruction_Tests)

BOOST_AUTO_TEST_CASE(destroy_threadpool_with_nonempty_queue)
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
  BOOST_CHECK(t.timed_join(long_timeout));
  BOOST_REQUIRE(boost::thread::id() == t.get_id());
  b.V();
  BOOST_CHECK(!c.P(long_timeout));
}

BOOST_AUTO_TEST_CASE(destroy_threadpool_with_nonempty_queue_with_completeAllJobsBeforeDestruction_true)
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
  BOOST_CHECK(t.timed_join(long_timeout));
  BOOST_REQUIRE(boost::thread::id() == t.get_id());
  b.V();
  BOOST_CHECK(c.P(long_timeout));
}

BOOST_AUTO_TEST_SUITE_END()
