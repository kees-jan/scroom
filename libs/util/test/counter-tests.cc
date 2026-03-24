/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <memory>

#include <gtest/gtest.h>

#include <scroom/utilities.hh>

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

class TestCounted : public Counted<TestCounted>
{
};

//////////////////////////////////////////////////////////////

TEST(Counter_Tests, count) // NOLINT
{
  Counter* counter = Counter::instance();
  const std::string testCountedName = typeid(TestCounted).name();
  std::list<Count::Ptr> counts = counter->getCounts();
  EXPECT_EQ(0, counts.size());
  Count::Ptr c;

  {
    TestCounted const t;
    counts = counter->getCounts();
    ASSERT_EQ(1, counts.size());
    c = counts.front();
    EXPECT_EQ(testCountedName, c->name);
    EXPECT_EQ(1, c->count);

    {
      TestCounted const t2;
      counts = counter->getCounts();
      ASSERT_EQ(1, counts.size());
      c = counts.front();
      EXPECT_EQ(testCountedName, c->name);
      EXPECT_EQ(2, c->count);
    }
    counts = counter->getCounts();
    ASSERT_EQ(1, counts.size());
    c = counts.front();
    EXPECT_EQ(testCountedName, c->name);
    EXPECT_EQ(1, c->count);
  }

  counts = counter->getCounts();
  ASSERT_EQ(1, counts.size());
  c = counts.front();
  EXPECT_EQ(testCountedName, c->name);
  EXPECT_EQ(0, c->count);
}
