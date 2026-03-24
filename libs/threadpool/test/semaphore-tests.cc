/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#include <gtest/gtest.h>

#include <scroom/function-additor.hh>
#include <scroom/semaphore.hh>

#include "helpers.hh"

using namespace Scroom;
using namespace boost::posix_time;

//////////////////////////////////////////////////////////////

void test_count_equals(Semaphore* s, int i)
{
  for(int actual = 0; actual < i; actual++)
  {
    ASSERT_TRUE(s->try_P()) << "Could only decrement " << actual << " times, instead of " << i;
  }
  ASSERT_FALSE(s->try_P()) << "Can decrement " << (i + 1) << " times, instead of " << i;
}

//////////////////////////////////////////////////////////////

TEST(Semaphore_Tests, try_P) // NOLINT
{
  Semaphore s1(0);
  ASSERT_FALSE(s1.try_P());
  Semaphore s2(1);
  ASSERT_TRUE(s2.try_P());
  ASSERT_FALSE(s2.try_P());
}

TEST(Semaphore_Tests, count_equals_0) // NOLINT
{
  Semaphore s(0);
  test_count_equals(&s, 0);
}

TEST(Semaphore_Tests, count_equals_1) // NOLINT
{
  Semaphore s(1);
  test_count_equals(&s, 1);
}

TEST(Semaphore_Tests, count_equals_2) // NOLINT
{
  Semaphore s(2);
  test_count_equals(&s, 2);
}

TEST(Semaphore_Tests, inc_count_1) // NOLINT
{
  Semaphore s(0);
  s.V();
  test_count_equals(&s, 1);
}

TEST(Semaphore_Tests, inc_count_2) // NOLINT
{
  Semaphore s(0);
  s.V();
  s.V();
  test_count_equals(&s, 2);
}

TEST(Semaphore_Tests, inc_count_3) // NOLINT
{
  Semaphore s(0);
  s.V();
  s.V();
  s.V();
  test_count_equals(&s, 3);
}

TEST(Semaphore_Tests, pass_many) // NOLINT
{
  Semaphore s(0);
  boost::thread t(5 * pass(&s));
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
  bool const success = boost::thread::id() == t.get_id();
  ASSERT_TRUE(success);
  if(!success)
  {
    t.interrupt();
    t.timed_join(millisec(250));
    ASSERT_TRUE(boost::thread::id() == t.get_id());
  }
}

TEST(Semaphore_Tests, p_with_timeout) // NOLINT
{
  Semaphore s(0);
  ASSERT_FALSE(s.P(millisec(250)));
  test_count_equals(&s, 0);
}

TEST(Semaphore_Tests, p_without_timeout) // NOLINT
{
  Semaphore s(1);
  ASSERT_TRUE(s.P(millisec(250)));
  ASSERT_FALSE(s.P(millisec(250)));
  test_count_equals(&s, 0);
}
