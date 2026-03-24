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

#include <memory>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

#include <gtest/gtest.h>

#include <scroom/function-additor.hh>
#include <scroom/semaphore.hh>

#include "helpers.hh"

using namespace boost::posix_time;
using namespace Scroom;

const millisec short_timeout(250);
const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

class A
{
private:
  Semaphore* s;

public:
  using Ptr = std::shared_ptr<A>;

  explicit A(Semaphore* s_)
    : s(s_)
  {
  }

  void operator()() { s->V(); }

  static Ptr create(Semaphore* s) { return std::make_shared<A>(s); }
};

template <typename R>
class B
{
private:
  Semaphore* s;
  R result;

public:
  using Ptr = std::shared_ptr<B<R>>;

  B(Semaphore* s_, R result_)
    : s(s_)
    , result(result_)
  {
  }

  R operator()()
  {
    s->V();
    return result;
  }

  static Ptr create(Semaphore* s, R result) { return Ptr(new B(s, result)); }
};

template <typename R>
R no_op(Semaphore* s, R result)
{
  s->V();
  return result;
}

//////////////////////////////////////////////////////////////

bool has_at_least_n_threads(ThreadPool* pool, int count_)
{
  if(count_ <= 0)
  {
    return true;
  }

  std::vector<Semaphore*> semaphores(count_);
  for(int i = 0; i < count_; i++)
  {
    semaphores[i] = new Semaphore(0);
  }

  for(int i = 0; i < count_ - 1; i++)
  {
    pool->schedule(pass(semaphores[i + 1]) + clear(semaphores[i]));
  }

  // All tasks are blocked on semaphores[count-1]

  pool->schedule(clear(semaphores[count_ - 1]));
  // If jobs of the same priority are scheduled in order, and if
  // there are at least count_ threads, then this final job will get
  // scheduled on the last available thread, thus freeing all
  // others.
  bool const result = semaphores[0]->P(long_timeout);

  if(!result)
  {
    // If there are too few threads, then all threads are still
    // blocked. This will ultimately block the ThreadPool destructor,
    // so we have to unblock them manually here.
    for(int i = 1; i < count_; i++)
    {
      semaphores[i]->V();
    }
  }
  return result;
}

bool has_exactly_n_threads(ThreadPool* pool, int count)
{
  return has_at_least_n_threads(pool, count) && !has_at_least_n_threads(pool, count + 1);
}

//////////////////////////////////////////////////////////////

TEST(ThreadPool_class_Tests, work_gets_done) // NOLINT
{
  Semaphore s(0);
  ThreadPool pool(0);
  pool.schedule(clear(&s));

  // Work doesn't get done with no threads
  EXPECT_FALSE(s.P(long_timeout));
  pool.add();

  // With a thread, work gets done
  EXPECT_TRUE(s.P(long_timeout));
}

TEST(ThreadPool_class_Tests, work_gets_done_by_prio) // NOLINT
{
  Semaphore high(0);
  Semaphore low(0);
  ThreadPool pool(0);
  pool.schedule(clear(&low), PRIO_NORMAL);
  pool.schedule(pass(&low) + clear(&high), PRIO_HIGH);

  pool.add();
  // Thread is doing the high-prio tasks first, which is blocked on
  // the low semaphore, hence, no work gets done.
  EXPECT_FALSE(high.P(short_timeout));

  pool.add();
  // Second thread does the low-prio task, which unblocks the
  // high-prio one. How's that for priority inversion :-)
  EXPECT_TRUE(high.P(long_timeout));
}

TEST(ThreadPool_class_Tests, construct_0_threads) // NOLINT
{
  ThreadPool pool(0);
  EXPECT_TRUE(has_exactly_n_threads(&pool, 0));
}

TEST(ThreadPool_class_Tests, construct_1_threads) // NOLINT
{
  ThreadPool pool(1);
  EXPECT_TRUE(has_exactly_n_threads(&pool, 1));
}

TEST(ThreadPool_class_Tests, construct_2_threads) // NOLINT
{
  ThreadPool pool(2);
  const int expected = 2;
#ifndef MULTITHREADING
  expected = 1;
#endif
  EXPECT_TRUE(has_exactly_n_threads(&pool, expected));
}

TEST(ThreadPool_class_Tests, schedule_shared_pointer) // NOLINT
{
  ThreadPool pool(1);
  Semaphore a(0);

  pool.schedule(A::create(&a));
  EXPECT_TRUE(a.P(long_timeout));
}

TEST(ThreadPool_class_Tests, schedule_future) // NOLINT
{
  ThreadPool pool(0);
  Semaphore a(0);

  boost::unique_future<int> result(pool.schedule<int>([pa = &a] { return no_op(pa, 42); }));

  EXPECT_FALSE(a.P(short_timeout));
  EXPECT_FALSE(result.is_ready());
  pool.add();

  EXPECT_TRUE(a.P(long_timeout));
  EXPECT_EQ(42, result.get());
}

TEST(ThreadPool_class_Tests, schedule_shared_pointer_with_future) // NOLINT
{
  ThreadPool pool(0);
  Semaphore a(0);

  boost::unique_future<bool> result(pool.schedule<bool, B<bool>>(B<bool>::create(&a, false)));

  EXPECT_FALSE(a.P(short_timeout));
  EXPECT_FALSE(result.is_ready());
  pool.add();

  EXPECT_TRUE(a.P(long_timeout));
  EXPECT_EQ(false, result.get());
}

//////////////////////////////////////////////////////////////

TEST(CpuBound_Tests, verify_threadcount) // NOLINT
{
  ThreadPool::Ptr const t = CpuBound();
  const int expected = boost::thread::hardware_concurrency();
#ifndef MULTITHREADING
  expected = 1;
#endif
  EXPECT_TRUE(has_exactly_n_threads(t.get(), expected));
}

//////////////////////////////////////////////////////////////

TEST(Sequentially_Tests, verify_threadcount) // NOLINT
{
  ThreadPool::Ptr const t = Sequentially();
  EXPECT_TRUE(has_exactly_n_threads(t.get(), 1));
}
