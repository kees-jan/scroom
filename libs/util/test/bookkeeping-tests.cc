/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gtest/gtest.h>

#include <scroom/bookkeeping.hh>

using namespace Scroom::Bookkeeping;

//////////////////////////////////////////////////////////////

TEST(Bookkeeping_Tests, token_arithmatic) // NOLINT
{
  Token a;
  Token b;
  Token c = a + b;
  WeakToken const wa(a);
  a.reset();
  EXPECT_TRUE(wa.lock());
  WeakToken const wb(b);
  b.reset();
  EXPECT_TRUE(wb.lock());
  c.reset();
  EXPECT_FALSE(wa.lock());
  EXPECT_FALSE(wb.lock());
}

TEST(Bookkeeping_Tests, basic_usage) // NOLINT
{
  Map<int, int>::Ptr map = Map<int, int>::create();
  ASSERT_TRUE(map);

  Token const a = map->reserve(1);
  map->at(1) = 1;
  Token b = map->reserve(2);
  map->at(2) = 2;

  EXPECT_TRUE(a);
  EXPECT_TRUE(b);
  EXPECT_EQ(1, int(map->at(1)));
  EXPECT_EQ(2, int(map->at(2)));
  EXPECT_EQ(1, map->get(1));
  EXPECT_EQ(2, map->get(2));
  EXPECT_EQ(2, map->keys().size());
  EXPECT_EQ(2, map->values().size());
  EXPECT_THROW(map->at(3), std::invalid_argument);
  EXPECT_THROW(map->reserve(2), std::invalid_argument);
  EXPECT_EQ(b, map->reReserve(2));
  EXPECT_EQ(2, map->get(2));
  EXPECT_EQ(1, map->get(1));
  map->set(2, 5);
  EXPECT_EQ(5, map->get(2));
  EXPECT_EQ(2, map->keys().size());
  EXPECT_EQ(2, map->values().size());
  b.reset();
  EXPECT_THROW(map->at(2), std::invalid_argument);
  EXPECT_EQ(1, map->get(1));
  EXPECT_EQ(1, map->keys().size());
  EXPECT_EQ(1, map->values().size());
  map.reset();
  EXPECT_TRUE(a);
}
