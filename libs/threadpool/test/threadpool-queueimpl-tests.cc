/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/threadpool.hh>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>

#include <scroom/semaphore.hh>
#include "helpers.hh"

using namespace boost::posix_time;
using namespace Scroom::Detail::ThreadPool;

const millisec short_timeout(250);
const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(ThreadPool_QueueImpl_Tests)

BOOST_AUTO_TEST_CASE(queueimpl_jobs_get_executed)
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  ThreadPool::WeakQueue::Ptr weak = queue->getWeak();
  Semaphore s(0);
  ThreadPool t(0);
  t.schedule(clear(&s), weak);
  t.add();
  BOOST_CHECK(s.P(long_timeout));
}

BOOST_AUTO_TEST_CASE(queueimpl_jobs_with_deleted_queue_can_be_scheduled_and_dont_get_executed)
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  ThreadPool::WeakQueue::Ptr weak = queue->getWeak();
  Semaphore s1(0);
  Semaphore s2(0);
  ThreadPool t(0);
  queue.reset();
  t.schedule(clear(&s1), weak);
  t.schedule(clear(&s2));
  t.add();
  BOOST_CHECK(s2.P(long_timeout));
  BOOST_CHECK(!s1.try_P());
}

BOOST_AUTO_TEST_SUITE_END()
