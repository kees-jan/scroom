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

#include "scroom/function-additor.hh"
#include "helpers.hh"

#include "queue.hh"

using namespace boost::posix_time;
using namespace Scroom::Detail::ThreadPool;

const millisec short_timeout(250);
const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(ThreadPool_Queue_Tests)

BOOST_AUTO_TEST_SUITE(Queue_Tests)

BOOST_AUTO_TEST_CASE(basic_jobcounting)
{
  QueueImpl::Ptr queue = QueueImpl::create();
  BOOST_CHECK(queue);
  BOOST_CHECK_EQUAL(0, queue->getCount());
  queue->jobStarted();
  BOOST_CHECK_EQUAL(1, queue->getCount());
  queue->jobStarted();
  BOOST_CHECK_EQUAL(2, queue->getCount());
  queue->jobFinished();
  BOOST_CHECK_EQUAL(1, queue->getCount());
  queue->jobFinished();
  BOOST_CHECK_EQUAL(0, queue->getCount());
}

BOOST_AUTO_TEST_CASE(destroy_waits_for_jobs_to_finish)
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  ThreadPool::Queue::WeakPtr weakQueue = queue;
  QueueImpl::Ptr qi = queue->get();
  BOOST_CHECK(queue);
  BOOST_CHECK(qi);
  BOOST_CHECK_EQUAL(0, qi->getCount());
  qi->jobStarted();
  BOOST_CHECK_EQUAL(1, qi->getCount());
  qi->jobStarted();
  BOOST_CHECK_EQUAL(2, qi->getCount());

  Semaphore s0(0);
  Semaphore s1(0);
  Semaphore s2(0);
  boost::thread t(pass(&s1)+destroy(queue)+clear(&s2));
  queue.reset();
  BOOST_CHECK(weakQueue.lock());
  s1.V();
  BOOST_REQUIRE(!s2.P(long_timeout));
  BOOST_CHECK(!weakQueue.lock());

  // At this point, all references to ThreadPool::Queue are gone, but the thread
  // trying to destroy it is blocked because
  // not all jobs have finished yet. So we should report the jobs complete,
  // and then the thread will unblock and the object will actually be deleted.
  qi->jobFinished();
  BOOST_REQUIRE(!s2.P(short_timeout));
  BOOST_CHECK_EQUAL(1, qi->getCount());
  qi->jobFinished();
  BOOST_REQUIRE(s2.P(long_timeout));
}

BOOST_AUTO_TEST_CASE(destroy_using_QueueLock)
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  ThreadPool::Queue::WeakPtr weakQueue = queue;
  BOOST_CHECK(queue);
  QueueLock* l = new QueueLock(queue->get());

  Semaphore s0(0);
  Semaphore s1(0);
  Semaphore s2(0);
  boost::thread t(clear(&s0)+pass(&s1)+destroy(queue)+clear(&s2));
  s0.P();
  BOOST_REQUIRE(!s2.P(short_timeout));
  queue.reset();
  BOOST_CHECK(weakQueue.lock());
  s1.V();
  BOOST_REQUIRE(!s2.P(long_timeout));
  BOOST_CHECK(!weakQueue.lock());

  // At this point, all references to ThreadPool::Queue are gone, but the thread
  // trying to destroy it is blocked because
  // not all jobs have finished yet. So we should report the jobs complete,
  // and then the thread will unblock and the object will actually be deleted.
  delete l;
  BOOST_REQUIRE(s2.P(long_timeout));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Queue_Tests)

BOOST_AUTO_TEST_CASE(jobs_on_custom_queue_get_executed)
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  Semaphore s(0);
  ThreadPool t(0, false);
  t.schedule(clear(&s), queue);
  t.add();
  BOOST_CHECK(s.P(long_timeout));
}

BOOST_AUTO_TEST_CASE(jobs_on_deleted_queue_dont_get_executed)
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  Semaphore s1(0);
  Semaphore s2(0);
  ThreadPool t(0, false);
  t.schedule(clear(&s1), queue);
  t.schedule(clear(&s2));
  queue.reset();
  t.add();
  BOOST_CHECK(s2.P(long_timeout));
  BOOST_CHECK(!s1.try_P());
}

BOOST_AUTO_TEST_CASE(queue_deletion_waits_for_jobs_to_finish)
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  ThreadPool::Queue::WeakPtr weakQueue = queue;
  Semaphore s0(0);
  Semaphore s1(0);
  Semaphore s2(0);
  Semaphore s3(0);
  Semaphore s4(0);

  ThreadPool pool(0, false);
  pool.schedule(clear(&s1)+pass(&s2), queue);
  pool.add();
  BOOST_REQUIRE(s1.P(long_timeout));
  // Job is now being executed, hence it should not be possible to delete the queue

  // Setup: Create a thread that will delete the queue. Then delete our
  // reference, because if our reference is the last, our thread will block,
  // resulting in deadlock
  boost::thread t(pass(&s3)+destroy(queue)+clear(&s4));
  queue.reset();

  // Tell the thread to start deleting the Queue
  s3.V();
  // Thread does not finish
  BOOST_CHECK(!s4.P(long_timeout));

  // Complete the job
  s2.V();
  // Thread now finishes throwing away the Queue
  BOOST_CHECK(s4.P(long_timeout));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
