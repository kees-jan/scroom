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
#include "scroom/function-additor.hh"

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
  typedef boost::shared_ptr<A> Ptr;

  A(Semaphore* s_)
    :s(s_)
  {}

  void operator()()
  {
    s->V();
  }

  static Ptr create(Semaphore* s)
  {
    return Ptr(new A(s));
  }
};

template<typename R>
class B
{
private:
  Semaphore* s;
  R result;

public:
  typedef boost::shared_ptr<B> Ptr;

  B(Semaphore* s_, R result_)
    :s(s_), result(result_)
  {}

  R operator()()
  {
    s->V();
    return result;
  }

  static Ptr create(Semaphore* s, R result)
  {
    return Ptr(new B(s, result));
  }
};

template<typename R>
R no_op(Semaphore* s, R result)
{
  s->V();
  return result;
}

//////////////////////////////////////////////////////////////

bool has_at_least_n_threads(ThreadPool* pool, int count_)
{
  if(count_ <= 0)
    return true;
  else
  {
    std::vector<Semaphore*> semaphores(count_);
    for(int i=0; i<count_; i++)
      semaphores[i] = new Semaphore(0);

    for(int i=0; i<count_-1; i++)
      pool->schedule(pass(semaphores[i+1])+clear(semaphores[i]));

    // All tasks are blocked on semaphores[count-1]

    pool->schedule(clear(semaphores[count_-1]));
    // If jobs of the same priority are scheduled in order, and if
    // there are at least count_ threads, then this final job will get
    // scheduled on the last available thread, thus freeing all
    // others.
    bool result=semaphores[0]->P(long_timeout);

    if(!result)
    {
      // If there are too few threads, then all threads are still
      // blocked. This will ultimately block the ThreadPool destructor,
      // so we have to unblock them manually here.
      for(int i=1; i<count_; i++)
        semaphores[i]->V();
    }
    return result;
  }
}

bool has_exactly_n_threads(ThreadPool* pool, int count)
{
  return has_at_least_n_threads(pool, count) && !has_at_least_n_threads(pool, count+1);
}

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(ThreadPool_Tests)

BOOST_AUTO_TEST_SUITE(ThreadPool_class_Tests)

BOOST_AUTO_TEST_CASE(work_gets_done)
{
  Semaphore s(0);
  ThreadPool pool(0);
  pool.schedule(clear(&s));

  // Work doesn't get done with no threads
  BOOST_CHECK(!s.P(long_timeout));
  pool.add();

  // With a thread, work gets done
  BOOST_CHECK(s.P(long_timeout));
}

BOOST_AUTO_TEST_CASE(work_gets_done_by_prio)
{
  Semaphore high(0);
  Semaphore low(0);
  ThreadPool pool(0);
  pool.schedule(clear(&low), PRIO_NORMAL);
  pool.schedule(pass(&low)+clear(&high), PRIO_HIGH);

  pool.add();
  // Thread is doing the high-prio tasks first, which is blocked on
  // the low semaphore, hence, no work gets done.
  BOOST_CHECK(!high.P(short_timeout));

  pool.add();
  // Second thread does the low-prio task, which unblocks the
  // high-prio one. How's that for priority inversion :-)
  BOOST_CHECK(high.P(long_timeout));
}

BOOST_AUTO_TEST_CASE(construct_0_threads)
{
  ThreadPool pool(0);
  BOOST_CHECK(has_exactly_n_threads(&pool, 0));
}

BOOST_AUTO_TEST_CASE(construct_1_threads)
{
  ThreadPool pool(1);
  BOOST_CHECK(has_exactly_n_threads(&pool, 1));
}

BOOST_AUTO_TEST_CASE(construct_2_threads)
{
  ThreadPool pool(2);
  int expected = 2;
#ifndef MULTITHREADING
  expected=1;
#endif
  BOOST_CHECK(has_exactly_n_threads(&pool, expected));
}

BOOST_AUTO_TEST_CASE(schedule_shared_pointer)
{
  ThreadPool pool(1);
  Semaphore a(0);

  pool.schedule(A::create(&a));
  BOOST_CHECK(a.P(long_timeout));
}

BOOST_AUTO_TEST_CASE(schedule_future)
{
  ThreadPool pool(0);
  Semaphore a(0);

  boost::unique_future<int> result(pool.schedule<int>(boost::bind(no_op<int>, &a, 42)));

  BOOST_CHECK(!a.P(short_timeout));
  BOOST_CHECK(!result.is_ready());
  pool.add();

  BOOST_CHECK(a.P(long_timeout));
  BOOST_CHECK_EQUAL(42, result.get());
}

BOOST_AUTO_TEST_CASE(schedule_shared_pointer_with_future)
{
  ThreadPool pool(0);
  Semaphore a(0);

  boost::unique_future<bool> result(pool.schedule<bool,B<bool> >(B<bool>::create(&a, false)));

  BOOST_CHECK(!a.P(short_timeout));
  BOOST_CHECK(!result.is_ready());
  pool.add();

  BOOST_CHECK(a.P(long_timeout));
  BOOST_CHECK_EQUAL(false, result.get());
}

BOOST_AUTO_TEST_SUITE_END()

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(CpuBound_Tests)

BOOST_AUTO_TEST_CASE(verify_threadcount)
{
  ThreadPool::Ptr t = CpuBound();
  int expected = boost::thread::hardware_concurrency();
#ifndef MULTITHREADING
  expected=1;
#endif
  BOOST_CHECK(has_exactly_n_threads(t.get(), expected));
}

BOOST_AUTO_TEST_SUITE_END()

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Sequentially_Tests)

BOOST_AUTO_TEST_CASE(verify_threadcount)
{
  ThreadPool::Ptr t = Sequentially();
  BOOST_CHECK(has_exactly_n_threads(t.get(), 1));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
