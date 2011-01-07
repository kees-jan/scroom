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

using namespace boost::posix_time;
using namespace Scroom;

const millisec short_timeout(250);
const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

class TestQueue : public ThreadPool::Queue
{
public:
  typedef boost::shared_ptr<TestQueue> Ptr;
  typedef boost::weak_ptr<TestQueue> WeakPtr;
  
  static Ptr create() { return Ptr(new TestQueue()); }
  
  TestQueue() {}
  void jobStarted() { ThreadPool::Queue::jobStarted(); }
  void jobFinished() { ThreadPool::Queue::jobFinished(); }
  unsigned int getCount() { return count; }
};

//////////////////////////////////////////////////////////////

static void pass_destroy_and_clear(Semaphore* s1, TestQueue::WeakPtr q, Semaphore* s2)
{
  TestQueue::Ptr queue(q);
  s1->P();
  queue.reset();
  s2->V();
}

static void clear_sem(Semaphore* s)
{
  s->V();
}

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(ThreadPool_Queue_Tests)

BOOST_AUTO_TEST_SUITE(Queue_Tests)

BOOST_AUTO_TEST_CASE(basic_jobcounting)
{
  TestQueue::Ptr queue = TestQueue::create();
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
  TestQueue::Ptr queue = TestQueue::create();
  TestQueue::WeakPtr weakQueue = queue;
  BOOST_CHECK(queue);
  BOOST_CHECK_EQUAL(0, queue->getCount());
  queue->jobStarted();
  BOOST_CHECK_EQUAL(1, queue->getCount());
  queue->jobStarted();
  BOOST_CHECK_EQUAL(2, queue->getCount());

  Semaphore s1(0);
  Semaphore s2(0);
  boost::thread t(boost::bind(pass_destroy_and_clear, &s1, weakQueue, &s2));
  BOOST_REQUIRE(!s2.P(short_timeout));
  TestQueue* pq = queue.get();
  queue.reset();
  BOOST_CHECK(weakQueue.lock());
  s1.V();
  BOOST_REQUIRE(!s2.P(long_timeout));
  BOOST_CHECK(!weakQueue.lock());

  // At this point, all references to TestQueue are gone, but the object
  // is still there. The thread trying to destroy it is blocked because
  // not all jobs have finished yet. So we should report the jobs complete,
  // and then the thread will unblock and the object will actually be deleted.
  pq->jobFinished();
  BOOST_REQUIRE(!s2.P(short_timeout));
  BOOST_CHECK_EQUAL(1, pq->getCount());
  pq->jobFinished();
  BOOST_REQUIRE(s2.P(long_timeout));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Queue_Tests)

BOOST_AUTO_TEST_CASE(jobs_on_custom_queue_get_executed)
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  Semaphore s(0);
  ThreadPool t(0);
  t.schedule(boost::bind(clear_sem, &s), queue);
  BOOST_CHECK(!s.P(short_timeout));
  t.add();
  BOOST_CHECK(s.P(long_timeout));
}

BOOST_AUTO_TEST_CASE(jobs_on_deleted_queue_dont_get_executed)
{
  ThreadPool::Queue::Ptr queue = ThreadPool::Queue::create();
  Semaphore s(0);
  ThreadPool t(0);
  t.schedule(boost::bind(clear_sem, &s), queue);
  BOOST_CHECK(!s.P(short_timeout));
  queue.reset();
  t.add();
  BOOST_CHECK(!s.P(long_timeout));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
