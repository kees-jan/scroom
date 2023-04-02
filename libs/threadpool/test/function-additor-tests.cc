/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <scroom/function-additor.hh>
#include <scroom/semaphore.hh>

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
  clear (&s)();
  BOOST_REQUIRE(s.try_P());
}

BOOST_AUTO_TEST_CASE(pass_passes)
{
  Semaphore s(1);
  pass (&s)();
  BOOST_REQUIRE(!s.try_P());
}

BOOST_AUTO_TEST_CASE(destroy_destroys)
{
  std::shared_ptr<int>     p(new int(4));
  std::weak_ptr<int> const w(p);
  BOOST_REQUIRE(w.lock());

  boost::function<void()> const f(destroy(p));
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
  Semaphore                                   s1(0);
  Semaphore                                   s2(0);
  a += pass(&s1);
  BOOST_CHECK_EQUAL(&a, &(a + clear(&s2)));

  boost::thread t(a);
  BOOST_CHECK(!s2.P(short_timeout));
  s1.V();
  BOOST_CHECK(s2.P(long_timeout));
  BOOST_CHECK(t.timed_join(long_timeout));
}

BOOST_AUTO_TEST_CASE(right_association)
{
  Scroom::Detail::ThreadPool::FunctionAdditor a;
  Semaphore                                   s1(0);
  Semaphore                                   s2(0);
  a += clear(&s2);

  BOOST_CHECK_EQUAL(&a, &(pass(&s1) + a));
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

  (5 * pass(&s))();
  BOOST_REQUIRE(!s.try_P());

  Semaphore s2(25);
  ((5 * pass(&s2)) * 5)();
  BOOST_REQUIRE(!s2.try_P());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Combined_Tests)

BOOST_AUTO_TEST_CASE(Test_If_Expressions_Compile)
{
  (void)(clear(nullptr) + 5 * clear(nullptr));
  (void)(clear(nullptr) + (5 * clear(nullptr)) * 5);
  (void)(clear(nullptr) + 5 * (5 * clear(nullptr)));
  (void)(4 * (clear(nullptr) + clear(nullptr)));
  (void)((clear(nullptr) + clear(nullptr)) * 4);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
