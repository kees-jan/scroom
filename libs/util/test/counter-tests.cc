/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/shared_ptr.hpp>
#include <boost/test/unit_test.hpp>

#include <scroom/utilities.hh>

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

class TestCounted : public Counted<TestCounted>
{
};

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Counter_Tests)

BOOST_AUTO_TEST_CASE(count)
{
  Counter*              counter         = Counter::instance();
  std::string           testCountedName = typeid(TestCounted).name();
  std::list<Count::Ptr> counts          = counter->getCounts();
  BOOST_CHECK_EQUAL(0, counts.size());
  Count::Ptr c;

  {
    TestCounted t;
    counts = counter->getCounts();
    BOOST_REQUIRE_EQUAL(1, counts.size());
    c = counts.front();
    BOOST_CHECK_EQUAL(testCountedName, c->name);
    BOOST_CHECK_EQUAL(1, c->count);

    {
      TestCounted t2;
      counts = counter->getCounts();
      BOOST_REQUIRE_EQUAL(1, counts.size());
      c = counts.front();
      BOOST_CHECK_EQUAL(testCountedName, c->name);
      BOOST_CHECK_EQUAL(2, c->count);
    }
    counts = counter->getCounts();
    BOOST_REQUIRE_EQUAL(1, counts.size());
    c = counts.front();
    BOOST_CHECK_EQUAL(testCountedName, c->name);
    BOOST_CHECK_EQUAL(1, c->count);
  }

  counts = counter->getCounts();
  BOOST_REQUIRE_EQUAL(1, counts.size());
  c = counts.front();
  BOOST_CHECK_EQUAL(testCountedName, c->name);
  BOOST_CHECK_EQUAL(0, c->count);
}

BOOST_AUTO_TEST_SUITE_END()
