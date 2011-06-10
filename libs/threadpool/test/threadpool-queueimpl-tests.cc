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
  Semaphore s(0);
  ThreadPool t(0);
  queue.reset();
  t.schedule(clear(&s), weak);
  t.add();
  BOOST_CHECK(!s.P(long_timeout));
}

BOOST_AUTO_TEST_SUITE_END()