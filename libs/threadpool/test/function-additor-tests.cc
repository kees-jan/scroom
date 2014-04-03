/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#include <scroom/semaphore.hh>

#include "scroom/function-additor.hh"
#include "helpers.hh"

using namespace boost::posix_time;
using namespace Scroom;

static const millisec short_timeout(250);
static const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Function_Additor_Tests)

BOOST_AUTO_TEST_SUITE(Helper_Tests)

BOOST_AUTO_TEST_CASE(clear_clears)
{
  Semaphore s(0);
  BOOST_REQUIRE(!s.try_P());
  clear(&s)();
  BOOST_REQUIRE(s.try_P());
}

BOOST_AUTO_TEST_CASE(pass_passes)
{
  Semaphore s(1);
  pass(&s)();
  BOOST_REQUIRE(!s.try_P());
}

BOOST_AUTO_TEST_CASE(destroy_destroys)
{
  boost::shared_ptr<int> p(new int(4));
  boost::weak_ptr<int> w(p);
  BOOST_REQUIRE(w.lock());

  boost::function<void ()> f(destroy(p));
  p.reset();
  BOOST_REQUIRE(w.lock());

  f();
  BOOST_REQUIRE(!w.lock());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Additor_Tests)

BOOST_AUTO_TEST_CASE(additor_adds)
{
  Semaphore s1(0);
  Semaphore s2(0);
  BOOST_REQUIRE(!s1.try_P());
  BOOST_REQUIRE(!s2.try_P());
  (clear(&s1) + clear(&s2))();
  BOOST_REQUIRE(s1.try_P());
  BOOST_REQUIRE(s2.try_P());
}

BOOST_AUTO_TEST_CASE(order_is_preserved)
{
  Semaphore s1(0);
  Semaphore s2(0);
  BOOST_CHECK(!s1.try_P());
  BOOST_CHECK(!s2.try_P());
  boost::thread t(pass(&s1) + clear(&s2));
  BOOST_CHECK(!s2.P(short_timeout));
  s1.V();
  BOOST_CHECK(s2.P(long_timeout));
  BOOST_CHECK(t.timed_join(long_timeout));
}

BOOST_AUTO_TEST_CASE(left_association)
{
  Scroom::Detail::ThreadPool::FunctionAdditor a;
  Semaphore s1(0);
  Semaphore s2(0);
  a += pass(&s1);
  BOOST_CHECK_EQUAL(&a, &(a+clear(&s2)));
  
  boost::thread t(a);
  BOOST_CHECK(!s2.P(short_timeout));
  s1.V();
  BOOST_CHECK(s2.P(long_timeout));
  BOOST_CHECK(t.timed_join(long_timeout));
}

BOOST_AUTO_TEST_CASE(right_association)
{
  Scroom::Detail::ThreadPool::FunctionAdditor a;
  Semaphore s1(0);
  Semaphore s2(0);
  a += clear(&s2);

  BOOST_CHECK_EQUAL(&a, &(pass(&s1)+a));
  boost::thread t(a);
  BOOST_CHECK(!s2.P(short_timeout));
  s1.V();
  BOOST_CHECK(s2.P(long_timeout));
  BOOST_CHECK(t.timed_join(long_timeout));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Multiplier_Tests)

BOOST_AUTO_TEST_CASE(Multiplier_multiplies)
{
  Semaphore s(5);

  (5*pass(&s))();
  BOOST_REQUIRE(!s.try_P());

  Semaphore s2(25);
  ((5*pass(&s2))*5)();
  BOOST_REQUIRE(!s2.try_P());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Combined_Tests)

BOOST_AUTO_TEST_CASE(Test_If_Expressions_Compile)
{
  (void)(clear(NULL)+5*clear(NULL));
  (void)(clear(NULL)+(5*clear(NULL))*5);
  (void)(clear(NULL)+5*(5*clear(NULL)));
  (void)(4*(clear(NULL)+clear(NULL)));
  (void)((clear(NULL)+clear(NULL))*4);
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
