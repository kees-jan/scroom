/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>

#include <gtest/gtest.h>

#include <scroom/function-additor.hh>
#include <scroom/semaphore.hh>

#include "helpers.hh"

using namespace boost::posix_time;
using namespace Scroom;

static const millisec short_timeout(250);
static const millisec long_timeout(2000);

//////////////////////////////////////////////////////////////

TEST(Function_Additor_Helper_Tests, clear_clears) // NOLINT
{
  Semaphore s(0);
  ASSERT_FALSE(s.try_P());
  clear (&s)();
  ASSERT_TRUE(s.try_P());
}

TEST(Function_Additor_Helper_Tests, pass_passes) // NOLINT
{
  Semaphore s(1);
  pass (&s)();
  ASSERT_FALSE(s.try_P());
}

TEST(Function_Additor_Helper_Tests, destroy_destroys) // NOLINT
{
  std::shared_ptr<int> p(new int(4));
  std::weak_ptr<int> const w(p);
  ASSERT_TRUE(w.lock());

  boost::function<void()> const f(destroy(p));
  p.reset();
  ASSERT_TRUE(w.lock());

  f();
  ASSERT_FALSE(w.lock());
}

TEST(Function_Additor_Additor_Tests, additor_adds) // NOLINT
{
  Semaphore s1(0);
  Semaphore s2(0);
  ASSERT_FALSE(s1.try_P());
  ASSERT_FALSE(s2.try_P());
  (clear(&s1) + clear(&s2))();
  ASSERT_TRUE(s1.try_P());
  ASSERT_TRUE(s2.try_P());
}

TEST(Function_Additor_Additor_Tests, order_is_preserved) // NOLINT
{
  Semaphore s1(0);
  Semaphore s2(0);
  EXPECT_FALSE(s1.try_P());
  EXPECT_FALSE(s2.try_P());
  boost::thread t(pass(&s1) + clear(&s2));
  EXPECT_FALSE(s2.P(short_timeout));
  s1.V();
  EXPECT_TRUE(s2.P(long_timeout));
  EXPECT_TRUE(t.timed_join(long_timeout));
}

TEST(Function_Additor_Additor_Tests, left_association) // NOLINT
{
  Scroom::Detail::ThreadPool::FunctionAdditor a;
  Semaphore s1(0);
  Semaphore s2(0);
  a += pass(&s1);
  EXPECT_EQ(&a, &(a + clear(&s2)));

  boost::thread t(a);
  EXPECT_FALSE(s2.P(short_timeout));
  s1.V();
  EXPECT_TRUE(s2.P(long_timeout));
  EXPECT_TRUE(t.timed_join(long_timeout));
}

TEST(Function_Additor_Additor_Tests, right_association) // NOLINT
{
  Scroom::Detail::ThreadPool::FunctionAdditor a;
  Semaphore s1(0);
  Semaphore s2(0);
  a += clear(&s2);

  EXPECT_EQ(&a, &(pass(&s1) + a));
  boost::thread t(a);
  EXPECT_FALSE(s2.P(short_timeout));
  s1.V();
  EXPECT_TRUE(s2.P(long_timeout));
  EXPECT_TRUE(t.timed_join(long_timeout));
}

TEST(Function_Additor_Multiplier_Tests, Multiplier_multiplies) // NOLINT
{
  Semaphore s(5);

  (5 * pass(&s))();
  ASSERT_FALSE(s.try_P());

  Semaphore s2(25);
  ((5 * pass(&s2)) * 5)();
  ASSERT_FALSE(s2.try_P());
}

TEST(Function_Additor_Combined_Tests, Test_If_Expressions_Compile) // NOLINT
{
  (void)(clear(nullptr) + 5 * clear(nullptr));
  (void)(clear(nullptr) + (5 * clear(nullptr)) * 5);
  (void)(clear(nullptr) + 5 * (5 * clear(nullptr)));
  (void)(4 * (clear(nullptr) + clear(nullptr)));
  (void)((clear(nullptr) + clear(nullptr)) * 4);
}
