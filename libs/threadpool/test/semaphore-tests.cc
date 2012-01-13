/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

#include <scroom/semaphore.hh>

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "helpers.hh"
#include "function-additor.hh"

using namespace Scroom;
using namespace boost::posix_time;

//////////////////////////////////////////////////////////////

void test_count_equals(Semaphore* s, int i)
{
  for(int actual=0; actual<i; actual++)
  {
    BOOST_REQUIRE_MESSAGE(s->try_P(), "Could only decrement " << actual << " times, instead of " << i);
  }
  BOOST_REQUIRE_MESSAGE(!s->try_P(), "Can decrement " << (i+1) << " times, instead of " << i);
}

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Semaphore_Tests)

BOOST_AUTO_TEST_CASE(try_P)
{
  Semaphore s1(0);
  BOOST_REQUIRE(!s1.try_P());
  Semaphore s2(1);
  BOOST_REQUIRE(s2.try_P());
  BOOST_REQUIRE(!s2.try_P());
}

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
  boost::thread t(5*pass(&s));
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
  BOOST_REQUIRE(success);
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
  BOOST_REQUIRE(!s.P(millisec(250)));
  test_count_equals(&s, 0);
}

BOOST_AUTO_TEST_CASE(p_without_timeout)
{
  Semaphore s(1);
  BOOST_REQUIRE(s.P(millisec(250)));
  BOOST_REQUIRE(!s.P(millisec(250)));
  test_count_equals(&s, 0);
}

BOOST_AUTO_TEST_SUITE_END()
