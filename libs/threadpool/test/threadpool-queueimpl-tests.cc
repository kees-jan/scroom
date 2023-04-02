/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/threadpool.hh>

#include <iostream>
#include <memory>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#include <gtest/gtest.h>

#include <scroom/semaphore.hh>

#include "helpers.hh"

using namespace boost::posix_time;
using namespace Scroom::Detail::ThreadPool;

[[maybe_unused]]
const millisec short_timeout(250);
const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

TEST(ThreadPool_QueueImpl_Tests, queueimpl_jobs_get_executed) // NOLINT
{
  ThreadPool::Queue::Ptr const queue = ThreadPool::Queue::create();
  ThreadPool::WeakQueue::Ptr const weak = queue->getWeak();
  Semaphore s(0);
  ThreadPool t(0);
  t.schedule(clear(&s), weak);
  t.add();
  EXPECT_TRUE(s.P(long_timeout));
}

TEST(ThreadPool_QueueImpl_Tests, queueimpl_jobs_with_deleted_queue_can_be_scheduled_and_dont_get_executed) // NOLINT
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  ThreadPool::WeakQueue::Ptr const weak = queue->getWeak();
  Semaphore s1(0);
  Semaphore s2(0);
  ThreadPool t(0);
  queue.reset();
  t.schedule(clear(&s1), weak);
  t.schedule(clear(&s2));
  t.add();
  EXPECT_TRUE(s2.P(long_timeout));
  EXPECT_FALSE(s1.try_P());
}
