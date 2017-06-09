/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/threadpool.hh>

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

//////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, PriorityRange const& range)
{
  return os << '[' << range.highest << ',' << range.lowest << ']';
}

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(PriorityRange_Tests)

BOOST_AUTO_TEST_CASE(relational_operators)
{
  PriorityRange p1(0, 5);
  PriorityRange p2(5, 6);
  PriorityRange p3(6, 7);

  BOOST_CHECK(! (p1<p2));
  BOOST_CHECK(! (p1>p2));
  BOOST_CHECK_LT(p1, p3);

  BOOST_CHECK_LT( -1, p1);
  BOOST_CHECK(! (0 < p1));
  BOOST_CHECK_LT(p1, 6);
  BOOST_CHECK(! (p1<5));

  BOOST_CHECK_NE(p1, p2);
  BOOST_CHECK_NE(p1, p3);
  BOOST_CHECK_EQUAL(p1, p1);
}

BOOST_AUTO_TEST_CASE(dispenser)
{
  PriorityRangeDispenser::Ptr d = PriorityRangeDispenser::create(0);
  BOOST_CHECK_EQUAL(d->get(2), PriorityRange(0,1));
  BOOST_CHECK_EQUAL(d->get(3), PriorityRange(2,4));
  BOOST_CHECK_EQUAL(d->get(1), PriorityRange(5,5));
}

BOOST_AUTO_TEST_CASE(dispenser_overflow)
{
  const int base = 0x70000000;
  const int r1 = 0x09000000;
  const int r2 = 0x0A000000; // base + r1 +r2 triggers integer overflow

  // BOOST_CHECK_LT(base+r1+r2, base); // Triggers a compiler warning for (intended) integer overflow

  PriorityRangeDispenser::Ptr d = PriorityRangeDispenser::create(base);
  
  BOOST_CHECK_EQUAL(d->get(r1), PriorityRange(base, base+r1-1));
  BOOST_CHECK_EQUAL(d->get(r2), PriorityRange(base, base+r2-1));
}

BOOST_AUTO_TEST_SUITE_END()
