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
#include "queue.hh"

using namespace boost::posix_time;
using namespace Scroom::Detail::ThreadPool;

static const millisec short_timeout(250);
static const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

TEST(ThreadPool_Queue_QueueImpl, basic_jobcounting) // NOLINT
{
  QueueImpl::Ptr const queue = QueueImpl::create();
  EXPECT_TRUE(queue != nullptr);
  EXPECT_EQ(0, queue->getCount());
  queue->jobStarted();
  EXPECT_EQ(1, queue->getCount());
  queue->jobStarted();
  EXPECT_EQ(2, queue->getCount());
  queue->jobFinished();
  EXPECT_EQ(1, queue->getCount());
  queue->jobFinished();
  EXPECT_EQ(0, queue->getCount());
}

TEST(ThreadPool_Queue_QueueImpl, destroy_waits_for_jobs_to_finish) // NOLINT
{
  ThreadPool::Queue::Ptr           queue     = ThreadPool::Queue::create();
  ThreadPool::Queue::WeakPtr const weakQueue = queue;
  QueueImpl::Ptr const             qi        = queue->get();
  EXPECT_TRUE(queue != nullptr);
  EXPECT_TRUE(qi != nullptr);
  EXPECT_EQ(0, qi->getCount());
  qi->jobStarted();
  EXPECT_EQ(1, qi->getCount());
  qi->jobStarted();
  EXPECT_EQ(2, qi->getCount());

  Semaphore           s1(0);
  Semaphore           s2(0);
  boost::thread const t(pass(&s1) + destroy(queue) + clear(&s2));
  queue.reset();
  EXPECT_TRUE(weakQueue.lock() != nullptr);
  s1.V();
  EXPECT_FALSE(s2.P(long_timeout));
  EXPECT_FALSE(weakQueue.lock() != nullptr);

  // At this point, all references to ThreadPool::Queue are gone, but the thread
  // trying to destroy it is blocked because
  // not all jobs have finished yet. So we should report the jobs complete,
  // and then the thread will unblock and the object will actually be deleted.
  qi->jobFinished();
  EXPECT_FALSE(s2.P(short_timeout));
  EXPECT_EQ(1, qi->getCount());
  qi->jobFinished();
  EXPECT_TRUE(s2.P(long_timeout));
}

TEST(ThreadPool_Queue_QueueImpl, destroy_using_QueueLock) // NOLINT
{
  ThreadPool::Queue::Ptr           queue     = ThreadPool::Queue::create();
  ThreadPool::Queue::WeakPtr const weakQueue = queue;
  EXPECT_TRUE(queue != nullptr);
  auto* l = new QueueLock(queue->get());

  Semaphore           s0(0);
  Semaphore           s1(0);
  Semaphore           s2(0);
  boost::thread const t(clear(&s0) + pass(&s1) + destroy(queue) + clear(&s2));
  s0.P();
  EXPECT_FALSE(s2.P(short_timeout));
  queue.reset();
  EXPECT_TRUE(weakQueue.lock() != nullptr);
  s1.V();
  EXPECT_FALSE(s2.P(long_timeout));
  EXPECT_FALSE(weakQueue.lock() != nullptr);

  // At this point, all references to ThreadPool::Queue are gone, but the thread
  // trying to destroy it is blocked because
  // not all jobs have finished yet. So we should report the jobs complete,
  // and then the thread will unblock and the object will actually be deleted.
  delete l;
  EXPECT_TRUE(s2.P(long_timeout));
}

TEST(ThreadPool_Queue_Queue, jobs_on_custom_queue_get_executed) // NOLINT
{
  ThreadPool::Queue::Ptr const queue = ThreadPool::Queue::create();
  Semaphore                    s(0);
  ThreadPool                   t(0);
  t.schedule(clear(&s), queue);
  t.add();
  EXPECT_TRUE(s.P(long_timeout));
}

TEST(ThreadPool_Queue_Queue, jobs_on_deleted_queue_dont_get_executed) // NOLINT
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  Semaphore              s1(0);
  Semaphore              s2(0);
  ThreadPool             t(0);
  t.schedule(clear(&s1), queue);
  t.schedule(clear(&s2));
  queue.reset();
  t.add();
  EXPECT_TRUE(s2.P(long_timeout));
  EXPECT_FALSE(s1.try_P());
}

TEST(ThreadPool_Queue_Queue, queue_deletion_waits_for_jobs_to_finish) // NOLINT
{
  ThreadPool::Queue::Ptr           queue     = ThreadPool::Queue::create();
  ThreadPool::Queue::WeakPtr const weakQueue = queue;
  Semaphore                        s1(0);
  Semaphore                        s2(0);
  Semaphore                        s3(0);
  Semaphore                        s4(0);

  ThreadPool pool(0);
  pool.schedule(clear(&s1) + pass(&s2), queue);
  pool.add();
  EXPECT_TRUE(s1.P(long_timeout));
  // Job is now being executed, hence it should not be possible to delete the queue

  // Setup: Create a thread that will delete the queue. Then delete our
  // reference, because if our reference is the last, our thread will block,
  // resulting in deadlock
  boost::thread const t(pass(&s3) + destroy(queue) + clear(&s4));
  queue.reset();

  // Tell the thread to start deleting the Queue
  s3.V();
  // Thread does not finish
  EXPECT_FALSE(s4.P(long_timeout));

  // Complete the job
  s2.V();
  // Thread now finishes throwing away the Queue
  EXPECT_TRUE(s4.P(long_timeout));
}
