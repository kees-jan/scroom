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

#include <scroom/utilities.hh>

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

class TestCounted : public Counted<TestCounted>
{
};

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Counter_Tests)

BOOST_AUTO_TEST_CASE(count)
{
  Counter::Ptr counter = getCounter();
  std::string testCountedName = typeid(TestCounted).name();
  std::map<std::string, unsigned long*> counts = counter->getCounts();
  BOOST_CHECK(!counts.empty());
  BOOST_CHECK(counts.end() != counts.find(testCountedName));
  BOOST_CHECK_EQUAL(0, *counts[testCountedName]);

  {
    TestCounted t;
    counts = counter->getCounts();
    BOOST_CHECK(!counts.empty());
    BOOST_CHECK(counts.end() != counts.find(testCountedName));
    BOOST_CHECK_EQUAL(1, *counts[testCountedName]);
    {
      TestCounted t2;
      counts = counter->getCounts();
      BOOST_CHECK(!counts.empty());
      BOOST_CHECK(counts.end() != counts.find(testCountedName));
      BOOST_CHECK_EQUAL(2, *counts[testCountedName]);
    }
    counts = counter->getCounts();
    BOOST_CHECK(!counts.empty());
    BOOST_CHECK(counts.end() != counts.find(testCountedName));
    BOOST_CHECK_EQUAL(1, *counts[testCountedName]);
  }
  
  counts = counter->getCounts();
  BOOST_CHECK(!counts.empty());
  BOOST_CHECK(counts.end() != counts.find(testCountedName));
  BOOST_CHECK_EQUAL(0, *counts[testCountedName]);
}

BOOST_AUTO_TEST_SUITE_END()
