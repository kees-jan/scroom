#include <scroom/threadpool.hh>

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <scroom/semaphore.hh>

using namespace boost::posix_time;
using namespace Scroom;

const millisec short_timeout(250);
const millisec long_timeout(1000);

//////////////////////////////////////////////////////////////

void clear_sem(Semaphore* s)
{
  s->V();
}

void pass_and_clear(Semaphore* toPass, Semaphore* toClear)
{
  toPass->P();
  toClear->V();
}

void destroy(ThreadPool* threadpool)
{
  delete threadpool;
}

//////////////////////////////////////////////////////////////

bool has_at_least_n_threads(ThreadPool* pool, int count)
{
  if(count <= 0)
    return true;
  else
  {
    std::vector<Semaphore*> semaphores(count);
    for(int i=0; i<count; i++)
      semaphores[i] = new Semaphore(0);

    for(int i=0; i<count-1; i++)
      pool->schedule(boost::bind(pass_and_clear, semaphores[i+1], semaphores[i]));

    // All tasks are blocked on semaphores[count-1]
    BOOST_REQUIRE(!semaphores[0]->P(short_timeout));

    pool->schedule(boost::bind(clear_sem, semaphores[count-1]));
    // If jobs of the same priority are scheduled in order, and if
    // there are at least count threads, then this final job will get
    // scheduled on the last available thread, thus freeing all
    // others.
    bool result=semaphores[0]->P(long_timeout);

    if(!result)
    {
      // If there are too few threads, then all jobs are still
      // blocked. This will ultimately block the ThreadPool destructor,
      // so we have to unblock them manually here.
      for(int i=1; i<count; i++)
        semaphores[i]->V();

      // As a result, all semaphores should now be 1
      // Add an extra thread to cover the corner case that we had 0
      // threads to begin with
      ThreadPool::ThreadPtr t = pool->add();

      for(int i=0; i<count; i++)
      {
        BOOST_CHECK_MESSAGE(semaphores[i]->P(long_timeout), "Unblock " << i);
        delete semaphores[i];
        semaphores[i]=NULL;
      }

      // Terminate the thread we just added
      t->interrupt();
      t->timed_join(long_timeout);
      BOOST_REQUIRE(boost::thread::id() == t->get_id());
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

BOOST_AUTO_TEST_CASE(threads_terminate_on_destruction)
{
  ThreadPool* pool = new ThreadPool(0);
  ThreadPool::ThreadPtr t = pool->add();
  BOOST_CHECK(!t->timed_join(short_timeout));
  delete pool;

  bool success = boost::thread::id() == t->get_id();
  BOOST_CHECK(success);
  if(!success)
  {
    t->interrupt();
    t->timed_join(long_timeout);
    BOOST_REQUIRE(boost::thread::id() == t->get_id());
  }
}

BOOST_AUTO_TEST_CASE(threads_can_be_interrupted)
{
  ThreadPool pool(0);
  ThreadPool::ThreadPtr t = pool.add();
  boost::this_thread::sleep(millisec(50));
  t->interrupt();
  t->timed_join(short_timeout);
  BOOST_CHECK_EQUAL(boost::thread::id(), t->get_id());
}

BOOST_AUTO_TEST_CASE(work_gets_done)
{
  Semaphore s(0);
  ThreadPool pool(0);
  pool.schedule(boost::bind(clear_sem, &s));
  BOOST_CHECK(!s.P(short_timeout));
  pool.add();
  BOOST_CHECK(s.P(long_timeout));
}

BOOST_AUTO_TEST_CASE(work_gets_done_by_prio)
{
  Semaphore high(0);
  Semaphore low(0);
  ThreadPool pool(0);
  pool.schedule(boost::bind(clear_sem, &low), PRIO_NORMAL);
  pool.schedule(boost::bind(pass_and_clear, &low, &high), PRIO_HIGH);
  
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

BOOST_AUTO_TEST_CASE(destroy_threadpool_with_nonempty_queue)
{
  ThreadPool* pool = new ThreadPool(0);
  Semaphore dummy(0);

  pool->schedule(boost::bind(clear_sem, &dummy));

  boost::thread t(boost::bind(destroy, pool));
  t.timed_join(long_timeout);
  BOOST_REQUIRE(boost::thread::id() == t.get_id());
}

BOOST_AUTO_TEST_CASE(cant_destroy_threadpool_with_running_job)
{
  ThreadPool* pool = new ThreadPool(1);
  Semaphore a(0);
  Semaphore b(0);

  pool->schedule(boost::bind(pass_and_clear, &a, &b));

  // Give the thread some time to start the job
  boost::this_thread::sleep(millisec(50));
  
  boost::thread t(boost::bind(destroy, pool));
  BOOST_CHECK(!t.timed_join(short_timeout));
  a.V();
  t.timed_join(long_timeout);
  BOOST_REQUIRE(boost::thread::id() == t.get_id());
  BOOST_CHECK(b.P(short_timeout));
}

BOOST_AUTO_TEST_SUITE_END()

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(CpuBound_Tests)

BOOST_AUTO_TEST_CASE(verify_threadcount)
{
  ThreadPool::Ptr t = CpuBound::instance();
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
  ThreadPool::Ptr t = Sequentially::instance();
  BOOST_CHECK(has_exactly_n_threads(t.get(), 1));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
