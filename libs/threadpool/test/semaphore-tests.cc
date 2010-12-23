#include <scroom/semaphore.hh>

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace Scroom;
using namespace boost::posix_time;

//////////////////////////////////////////////////////////////

void passAndExit(Semaphore* s, int count=1)
{
  for(int i=0; i<count; i++)
    s->P();
}

void test_count_equals(Semaphore* s, int i)
{
  for(int actual=0; actual<i; actual++)
  {
    boost::thread t(boost::bind(passAndExit, s, 1));
    t.timed_join(millisec(250));
    bool succesfullyDecremented = boost::thread::id() == t.get_id();
    BOOST_CHECK_MESSAGE(succesfullyDecremented, "Could only decrement " << actual << " times, instead of " << i);

    if(!succesfullyDecremented)
    {
      t.interrupt();
      t.timed_join(millisec(250));
      BOOST_REQUIRE(boost::thread::id() == t.get_id());
      break;
    }
  }

  boost::thread t(boost::bind(passAndExit, s, 1));
  BOOST_CHECK(!t.timed_join(millisec(250)));
  t.interrupt();
  t.timed_join(millisec(250));
  BOOST_REQUIRE(boost::thread::id() == t.get_id());
}

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Semaphore_Tests)

BOOST_AUTO_TEST_CASE(count_equals_0)
{
  Semaphore s(0);
  test_count_equals(&s, 0);
}

BOOST_AUTO_TEST_CASE(count_equals_1)
{
  Semaphore s(1);
  test_count_equals(&s, 1);
}

BOOST_AUTO_TEST_CASE(count_equals_2)
{
  Semaphore s(2);
  test_count_equals(&s, 2);
}

BOOST_AUTO_TEST_CASE(inc_count_1)
{
  Semaphore s(0);
  s.V();
  test_count_equals(&s, 1);
}

BOOST_AUTO_TEST_CASE(inc_count_2)
{
  Semaphore s(0);
  s.V();
  s.V();
  test_count_equals(&s, 2);
}

BOOST_AUTO_TEST_CASE(inc_count_3)
{
  Semaphore s(0);
  s.V();
  s.V();
  s.V();
  test_count_equals(&s, 3);
}

BOOST_AUTO_TEST_CASE(pass_many)
{
  Semaphore s(0);
  boost::thread t(boost::bind(passAndExit, &s, 5));
  boost::this_thread::sleep(millisec(50));
  s.V();
  boost::this_thread::sleep(millisec(100));
  s.V();
  boost::this_thread::sleep(millisec(50));
  s.V();
  boost::this_thread::sleep(millisec(100));
  s.V();
  boost::this_thread::sleep(millisec(50));
  s.V();
  t.timed_join(millisec(250));
  bool success = boost::thread::id() == t.get_id();
  BOOST_CHECK(success);
  if(!success)
  {
    t.interrupt();
    t.timed_join(millisec(250));
    BOOST_REQUIRE(boost::thread::id() == t.get_id());
  }
}

BOOST_AUTO_TEST_CASE(p_with_timeout)
{
  Semaphore s(0);
  BOOST_CHECK(!s.P(millisec(250)));
  test_count_equals(&s, 0);
}

BOOST_AUTO_TEST_CASE(p_without_timeout)
{
  Semaphore s(1);
  BOOST_CHECK(s.P(millisec(250)));
  BOOST_CHECK(!s.P(millisec(250)));
  test_count_equals(&s, 0);
}

BOOST_AUTO_TEST_SUITE_END()
