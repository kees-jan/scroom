/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/test/unit_test.hpp>

#include <scroom/bookkeeping.hh>

using namespace Scroom::Bookkeeping;

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Bookkeeping_Tests)

BOOST_AUTO_TEST_CASE(token_arithmatic)
{
  Token           a;
  Token           b;
  Token           c = a + b;
  WeakToken const wa(a);
  a.reset();
  BOOST_CHECK(wa.lock());
  WeakToken const wb(b);
  b.reset();
  BOOST_CHECK(wb.lock());
  c.reset();
  BOOST_CHECK(!wa.lock());
  BOOST_CHECK(!wb.lock());
}

BOOST_AUTO_TEST_CASE(basic_usage)
{
  Map<int, int>::Ptr map = Map<int, int>::create();
  BOOST_REQUIRE(map);

  Token const a = map->reserve(1);
  map->at(1)    = 1;
  Token b       = map->reserve(2);
  map->at(2)    = 2;

  BOOST_CHECK(a);
  BOOST_CHECK(b);
  BOOST_CHECK_EQUAL(1, int(map->at(1)));
  BOOST_CHECK_EQUAL(2, int(map->at(2)));
  BOOST_CHECK_EQUAL(1, map->get(1));
  BOOST_CHECK_EQUAL(2, map->get(2));
  BOOST_CHECK_EQUAL(2, map->keys().size());
  BOOST_CHECK_EQUAL(2, map->values().size());
  BOOST_CHECK_THROW(map->at(3), std::invalid_argument);
  BOOST_CHECK_THROW(map->reserve(2), std::invalid_argument);
  BOOST_CHECK_EQUAL(b, map->reReserve(2));
  BOOST_CHECK_EQUAL(2, map->get(2));
  BOOST_CHECK_EQUAL(1, map->get(1));
  map->set(2, 5);
  BOOST_CHECK_EQUAL(5, map->get(2));
  BOOST_CHECK_EQUAL(2, map->keys().size());
  BOOST_CHECK_EQUAL(2, map->values().size());
  b.reset();
  BOOST_CHECK_THROW(map->at(2), std::invalid_argument);
  BOOST_CHECK_EQUAL(1, map->get(1));
  BOOST_CHECK_EQUAL(1, map->keys().size());
  BOOST_CHECK_EQUAL(1, map->values().size());
  map.reset();
  BOOST_CHECK(a);
}

BOOST_AUTO_TEST_SUITE_END()
